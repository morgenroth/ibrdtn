/*
 * DataReceiver.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Julian Timpner <timpner@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
package ibrdtn.api;

import ibrdtn.api.object.Block;
import ibrdtn.api.object.Bundle;
import ibrdtn.api.object.BundleID;
import ibrdtn.api.object.EID;
import ibrdtn.api.object.GroupEndpoint;
import ibrdtn.api.object.SingletonEndpoint;
import ibrdtn.api.sab.CallbackHandler;
import ibrdtn.api.sab.Response;
import ibrdtn.api.sab.SABException;
import ibrdtn.api.sab.SABHandler;
import ibrdtn.api.sab.SABParser;
import ibrdtn.api.sab.StatusReport;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.SynchronousQueue;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author Julian Timpner <timpner@ibr.cs.tu-bs.de>
 */
public class DataReceiver extends Thread implements SABHandler {

    private static final Logger logger = Logger.getLogger(DataReceiver.class.getName());
    private BlockingQueue<Response> queue = new SynchronousQueue<Response>();
    private BlockingQueue<List<String>> listqueue = new SynchronousQueue<List<String>>();
    private SABParser p = new SABParser();
    private final ExtendedClient client;
    private final Object handler_mutex;
    private final CallbackHandler handler;
    private Bundle current_bundle = null;
    private Block current_block = null;
    OutputStream outputStream = null;
    BufferedWriter outputWriter = null;
    CountingOutputStream counter = null;
    Long progress_last = 0L;
    private ProgressState progress_state = ProgressState.INITIAL;
    private boolean startBlockSent;

    public DataReceiver(ExtendedClient client, Object handler_mutex, CallbackHandler handler) {
        this.client = client;
        this.handler_mutex = handler_mutex;
        this.handler = handler;
    }

    private enum ProgressState {

        INITIAL, RECEIVING, DONE
    }

    @Override
    public void run() {
        try {
            p.parse(client.istream, this);
        } catch (SABException e) {
            logger.log(Level.SEVERE, "Parsing input stream failed", e);
            client.mark_error();
        }
    }

    public void abort() {
        p.abort();
        queue.offer(new Response(-1, ""));
        client.debug("abort queued");
    }

    /**
     * Reads the response to a command. If a notify is received first, it will call the listener to process the notify
     * message.
     *
     * @return the received return code
     * @throws IOException
     */
    public Response getResponse() throws APIException {
        try {
            Response obj = queue.take();
            if (obj.getCode() == -1) {
                throw new APIException("getResponse() failed: response was -1");
            }
            return obj;
        } catch (InterruptedException e) {
            throw new APIException("Interrupted");
        }
    }

    public List<String> getList() throws APIException {
        try {
            return listqueue.take();
        } catch (InterruptedException e) {
            throw new APIException("Interrupted");
        }
    }

    @Override
    public void startStream() {
    }

    @Override
    public void endStream() {
    }

    @Override
    public void startBundle() {
        logger.log(Level.SEVERE, "Starting bundle.");
        current_bundle = new Bundle();
    }

    @Override
    public void endBundle() {
        logger.log(Level.SEVERE, "Ending bundle.");
        synchronized (handler_mutex) {
            if (handler != null) {
                handler.endBundle();
            }
        }

        current_bundle = null;
    }

    @Override
    public void startBlock(Integer type) {
        logger.log(Level.SEVERE, "Starting block (type: {0})", type);

        /*
         * Current bundle is null if only the payload was requested
         */
        if (current_bundle != null) {
            current_block = Block.createBlock(type);

            synchronized (handler_mutex) {
                if (handler != null) {
                    handler.startBundle(current_bundle);
                }
            }
        }
    }

    @Override
    public void endBlock() {
        logger.log(Level.SEVERE, "Ending block.");

        if (!startBlockSent && current_block != null) {
            synchronized (handler_mutex) {
                if (handler != null) {
                    handler.startBlock(current_block);
                }
            }
        }

        // close outputWriter of the current block
        if (outputWriter != null) {
            try {
                outputWriter.flush();
                outputWriter.close();
            } catch (IOException e) {
                logger.log(Level.WARNING, "Failed to flush output writer", e);
            }

            outputWriter = null;
            counter = null;
        }

        if (outputStream != null) {
            try {
                outputStream.close();
            } catch (IOException e) {
                logger.log(Level.WARNING, "Failed to close output stream", e);
            }

            synchronized (handler_mutex) {
                if (handler != null) {
                    handler.endPayload();
                }
            }

            outputStream = null;
        }

        /*
         * Current block is null if only the payload was requested
         */
        if (current_block != null) {
            synchronized (handler_mutex) {
                if (handler != null) {
                    handler.endBlock();
                }
            }
        }

        // update progress stats
        progress_state = ProgressState.DONE;
        updateProgress();

        current_block = null;
        startBlockSent = false;
    }

    @Override
    public void attribute(String keyword, String value) {

        logger.log(Level.SEVERE, "Attribute: {0}={1}", new Object[]{keyword, value});

        if (current_bundle != null) {

            if (current_block == null) {

                if (keyword.equalsIgnoreCase("source")) {
                    current_bundle.setSource(new SingletonEndpoint(value));
                } else if (keyword.equalsIgnoreCase("destination")) {
                    /*
                     * This assumes that the processing flags have already been set.
                     */
                    EID dest;
                    if (current_bundle.isSingleton()) {
                        dest = new SingletonEndpoint(value);
                    } else {
                        dest = new GroupEndpoint(value);
                    }
                    current_bundle.setDestination(dest);
                } else if (keyword.equalsIgnoreCase("timestamp")) {
                    current_bundle.setTimestamp(Long.parseLong(value));
                } else if (keyword.equalsIgnoreCase("sequencenumber")) {
                    current_bundle.setSequencenumber(Long.parseLong(value));
                } else if (keyword.equalsIgnoreCase("Processing flags")) {
                    current_bundle.setProcflags(Long.parseLong(value));
                } else if (keyword.equalsIgnoreCase("Reportto")) {
                    if (!value.equals("dtn:none")) {
                        current_bundle.setReportto(new SingletonEndpoint(value));
                    }
                } else if (keyword.equalsIgnoreCase("Custodian")) {
                    if (!value.equals("dtn:none")) {
                        current_bundle.setCustodian(new SingletonEndpoint(value));
                    }
                } else if (keyword.equalsIgnoreCase("Lifetime")) {
                    current_bundle.setLifetime(Long.parseLong(value));
                }
            } else {

                if (keyword.equalsIgnoreCase("Length")) {
                    current_block.setLength(Long.parseLong(value));
                } else if (keyword.equalsIgnoreCase("Flags")) {
                    current_block.setFlags(value);
                }
            }

        }
    }

    private void initializeBlock() {
        /*
         * Current block is null if only the payload was requested
         */
        if (current_block != null) {
            synchronized (handler_mutex) {
                if (handler != null) {
                    handler.startBlock(current_block);
                }
            }
            startBlockSent = true;
        }

        synchronized (handler_mutex) {
            if (handler != null) {
                outputStream = handler.startPayload();
            }
        }

        /*
         * If outputStream is null it is assumed that the payload data is to be ignored.
         */
        if (outputStream != null) {
            counter = new CountingOutputStream(outputStream);
            outputWriter = new BufferedWriter(
                    new OutputStreamWriter(
                    new Base64.OutputStream(
                    counter,
                    Base64.DECODE)));
        }
    }

    @Override
    public void characters(String data) throws SABException {

        logger.log(Level.WARNING, "Characters: {0}", data);

        if (!startBlockSent) {
            initializeBlock();
        }

        if (outputWriter != null) {
            try {
                outputWriter.append(data);
            } catch (IOException e) {
                logger.log(Level.SEVERE, "Cannot write data to output stream.", e);
            }
        }

        // update progress stats
        updateProgress();
    }

    @Override
    public void notify(Integer type, String data) {
        logger.log(Level.SEVERE, "New notification {0}: {1}", new Object[]{String.valueOf(type), data});

        switch (type) {
            case 600: //COMMON
                logger.log(Level.SEVERE, "600 COMMON notification {0}", String.valueOf(type));
                break;
            case 601: // NEIGHBOR 
                logger.log(Level.SEVERE, "601 NEIGHBOR notification {0}", String.valueOf(type));
                break;
            case 602: // BUNDLE
                logger.log(Level.FINE, "New bundle notification {0}", String.valueOf(type));
                BundleID bundleID = parseBundleNotification(data);

                synchronized (handler_mutex) {
                    if (handler != null) {
                        handler.notify(bundleID);
                    }
                }

                break;
            case 603: // REPORT
                StatusReport report = new StatusReport(data);
                logger.log(Level.SEVERE, "New report {0}", report);

                synchronized (handler_mutex) {
                    if (handler != null) {
                        handler.notify(report);
                    }
                }
                break;
            case 604: // CUSTODY                
                logger.log(Level.SEVERE, "604 CUSTODY notification {0}", String.valueOf(type));
                break;
        }
    }

    @Override
    public void response(Integer type, String data) {
        client.debug("[Response] " + String.valueOf(type) + ", " + data);
        try {
            queue.put(new Response(type, data));
        } catch (InterruptedException e) {
        }
    }

    @Override
    public void response(List<String> data) {
        try {
            listqueue.put(data);
        } catch (InterruptedException e) {
        }
    }

    private void updateProgress() {
        if (current_block == null) {
            return;
        }
        if (current_block.getLength() == null) {
            return;
        }
        if (current_block.getType() != 1) {
            return;
        }

        switch (progress_state) {
            // initial state
            case INITIAL:
                // new block, announce zero
                synchronized (handler_mutex) {
                    if (handler != null) {
                        handler.progress(0, current_block.getLength());
                    }
                }

                progress_state = ProgressState.RECEIVING;
                progress_last = 0L;
                break;

            case RECEIVING:
                if (counter != null) {
                    long newcount = counter.getCount();
                    if (newcount != progress_last) {
                        // only announce if 5% has changed
                        if ((current_block.getLength() / 20) <= (newcount - progress_last)) {
                            synchronized (handler_mutex) {
                                if (handler != null) {
                                    handler.progress(newcount, current_block.getLength());
                                }
                            }

                            progress_last = newcount;
                        }
                    }
                }
                break;

            case DONE:
                synchronized (handler_mutex) {
                    if (handler != null) {
                        handler.progress(current_block.getLength(), current_block.getLength());
                    }
                }
                progress_state = ProgressState.INITIAL;
                break;
        }
    }

    private BundleID parseBundleNotification(String data) {
        BundleID bundleID = new BundleID();
        String[] tokens = data.split(" ");
        bundleID.setTimestamp(Long.parseLong(tokens[2]));
        bundleID.setSequenceNumber(Long.parseLong(tokens[3]));
        if (tokens.length == 6) {
            bundleID.setFragOffset(Long.parseLong(tokens[4]));
            bundleID.setSource(tokens[5]);
        } else if (tokens.length == 5) {
            bundleID.setSource(tokens[4]);
        }
        return bundleID;
    }
}