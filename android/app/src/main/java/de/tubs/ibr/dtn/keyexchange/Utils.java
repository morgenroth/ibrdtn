package de.tubs.ibr.dtn.keyexchange;

import java.math.BigInteger;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.os.Parcelable;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.service.DaemonService;

public class Utils {
	public static final String[][] PGP_WORD_LIST = {
		{"aardvark", "adroitness"},
		{"absurd", "adviser"},
		{"accrue", "aftermath"},
		{"acme", "aggregate"},
		{"adrift", "alkali"},
		{"adult", "almighty"},
		{"afflict", "amulet"},
		{"ahead", "amusement"},
		{"aimless", "antenna"},
		{"Algol", "applicant"},
		{"allow", "Apollo"},
		{"alone", "armistice"},
		{"ammo", "article"},
		{"ancient", "asteroid"},
		{"apple", "Atlantic"},
		{"artist", "atmosphere"},
		{"assume", "autopsy"},
		{"Athens", "Babylon"},
		{"atlas", "backwater"},
		{"Aztec", "barbecue"},
		{"baboon", "belowground"},
		{"backfield", "bifocals"},
		{"backward", "bodyguard"},
		{"banjo", "bookseller"},
		{"beaming", "borderline"},
		{"bedlamp", "bottomless"},
		{"beehive", "Bradbury"},
		{"beeswax", "bravado"},
		{"befriend", "Brazilian"},
		{"Belfast", "breakaway"},
		{"berserk", "Burlington"},
		{"billiard", "businessman"},
		{"bison", "butterfat"},
		{"blackjack", "Camelot"},
		{"blockade", "candidate"},
		{"blowtorch", "cannonball"},
		{"bluebird", "Capricorn"},
		{"bombast", "caravan"},
		{"bookshelf", "caretaker"},
		{"brackish", "celebrate"},
		{"breadline", "cellulose"},
		{"breakup", "certify"},
		{"brickyard", "chambermaid"},
		{"briefcase", "Cherokee"},
		{"Burbank", "Chicago"},
		{"button", "clergyman"},
		{"buzzard", "coherence"},
		{"cement", "combustion"},
		{"chairlift", "commando"},
		{"chatter", "company"},
		{"checkup", "component"},
		{"chisel", "concurrent"},
		{"choking", "confidence"},
		{"chopper", "conformist"},
		{"Christmas", "congregate"},
		{"clamshell", "consensus"},
		{"classic", "consulting"},
		{"classroom", "corporate"},
		{"cleanup", "corrosion"},
		{"clockwork", "councilman"},
		{"cobra", "crossover"},
		{"commence", "crucifix"},
		{"concert", "cumbersome"},
		{"cowbell", "customer"},
		{"crackdown", "Dakota"},
		{"cranky", "decadence"},
		{"crowfoot", "December"},
		{"crucial", "decimal"},
		{"crumpled", "designing"},
		{"crusade", "detector"},
		{"cubic", "detergent"},
		{"dashboard", "determine"},
		{"deadbolt", "dictator"},
		{"deckhand", "dinosaur"},
		{"dogsled", "direction"},
		{"dragnet", "disable"},
		{"drainage", "disbelief"},
		{"dreadful", "disruptive"},
		{"drifter", "distortion"},
		{"dropper", "document"},
		{"drumbeat", "embezzle"},
		{"drunken", "enchanting"},
		{"Dupont", "enrollment"},
		{"dwelling", "enterprise"},
		{"eating", "equation"},
		{"edict", "equipment"},
		{"egghead", "escapade"},
		{"eightball", "Eskimo"},
		{"endorse", "everyday"},
		{"endow", "examine"},
		{"enlist", "existence"},
		{"erase", "exodus"},
		{"escape", "fascinate"},
		{"exceed", "filament"},
		{"eyeglass", "finicky"},
		{"eyetooth", "forever"},
		{"facial", "fortitude"},
		{"fallout", "frequency"},
		{"flagpole", "gadgetry"},
		{"flatfoot", "Galveston"},
		{"flytrap", "getaway"},
		{"fracture", "glossary"},
		{"framework", "gossamer"},
		{"freedom", "graduate"},
		{"frighten", "gravity"},
		{"gazelle", "guitarist"},
		{"Geiger", "hamburger"},
		{"glitter", "Hamilton"},
		{"glucose", "handiwork"},
		{"goggles", "hazardous"},
		{"goldfish", "headwaters"},
		{"gremlin", "hemisphere"},
		{"guidance", "hesitate"},
		{"hamlet", "hideaway"},
		{"highchair", "holiness"},
		{"hockey", "hurricane"},
		{"indoors", "hydraulic"},
		{"indulge", "impartial"},
		{"inverse", "impetus"},
		{"involve", "inception"},
		{"island", "indigo"},
		{"jawbone", "inertia"},
		{"keyboard", "infancy"},
		{"kickoff", "inferno"},
		{"kiwi", "informant"},
		{"klaxon", "insincere"},
		{"locale", "insurgent"},
		{"lockup", "integrate"},
		{"merit", "intention"},
		{"minnow", "inventive"},
		{"miser", "Istanbul"},
		{"Mohawk", "Jamaica"},
		{"mural", "Jupiter"},
		{"music", "leprosy"},
		{"necklace", "letterhead"},
		{"Neptune", "liberty"},
		{"newborn", "maritime"},
		{"nightbird", "matchmaker"},
		{"Oakland", "maverick"},
		{"obtuse", "Medusa"},
		{"offload", "megaton"},
		{"optic", "microscope"},
		{"orca", "microwave"},
		{"payday", "midsummer"},
		{"peachy", "millionaire"},
		{"pheasant", "miracle"},
		{"physique", "misnomer"},
		{"playhouse", "molasses"},
		{"Pluto", "molecule"},
		{"preclude", "Montana"},
		{"prefer", "monument"},
		{"preshrunk", "mosquito"},
		{"printer", "narrative"},
		{"prowler", "nebula"},
		{"pupil", "newsletter"},
		{"puppy", "Norwegian"},
		{"python", "October"},
		{"quadrant", "Ohio"},
		{"quiver", "onlooker"},
		{"quota", "opulent"},
		{"ragtime", "Orlando"},
		{"ratchet", "outfielder"},
		{"rebirth", "Pacific"},
		{"reform", "pandemic"},
		{"regain", "Pandora"},
		{"reindeer", "paperweight"},
		{"rematch", "paragon"},
		{"repay", "paragraph"},
		{"retouch", "paramount"},
		{"revenge", "passenger"},
		{"reward", "pedigree"},
		{"rhythm", "Pegasus"},
		{"ribcage", "penetrate"},
		{"ringbolt", "perceptive"},
		{"robust", "performance"},
		{"rocker", "pharmacy"},
		{"ruffled", "phonetic"},
		{"sailboat", "photograph"},
		{"sawdust", "pioneer"},
		{"scallion", "pocketful"},
		{"scenic", "politeness"},
		{"scorecard", "positive"},
		{"Scotland", "potato"},
		{"seabird", "processor"},
		{"select", "provincial"},
		{"sentence", "proximate"},
		{"shadow", "puberty"},
		{"shamrock", "publisher"},
		{"showgirl", "pyramid"},
		{"skullcap", "quantity"},
		{"skydive", "racketeer"},
		{"slingshot", "rebellion"},
		{"slowdown", "recipe"},
		{"snapline", "recover"},
		{"snapshot", "repellent"},
		{"snowcap", "replica"},
		{"snowslide", "reproduce"},
		{"solo", "resistor"},
		{"southward", "responsive"},
		{"soybean", "retraction"},
		{"spaniel", "retrieval"},
		{"spearhead", "retrospect"},
		{"spellbind", "revenue"},
		{"spheroid", "revival"},
		{"spigot", "revolver"},
		{"spindle", "sandalwood"},
		{"spyglass", "sardonic"},
		{"stagehand", "Saturday"},
		{"stagnate", "savagery"},
		{"stairway", "scavenger"},
		{"standard", "sensation"},
		{"stapler", "sociable"},
		{"steamship", "souvenir"},
		{"sterling", "specialist"},
		{"stockman", "speculate"},
		{"stopwatch", "stethoscope"},
		{"stormy", "stupendous"},
		{"sugar", "supportive"},
		{"surmount", "surrender"},
		{"suspense", "suspicious"},
		{"sweatband", "sympathy"},
		{"swelter", "tambourine"},
		{"tactics", "telephone"},
		{"talon", "therapist"},
		{"tapeworm", "tobacco"},
		{"tempest", "tolerance"},
		{"tiger", "tomorrow"},
		{"tissue", "torpedo"},
		{"tonic", "tradition"},
		{"topmost", "travesty"},
		{"tracker", "trombonist"},
		{"transit", "truncated"},
		{"trauma", "typewriter"},
		{"treadmill", "ultimate"},
		{"Trojan", "undaunted"},
		{"trouble", "underfoot"},
		{"tumor", "unicorn"},
		{"tunnel", "unify"},
		{"tycoon", "universe"},
		{"uncut", "unravel"},
		{"unearth", "upcoming"},
		{"unwind", "vacancy"},
		{"uproot", "vagabond"},
		{"upset", "vertigo"},
		{"upshot", "Virginia"},
		{"vapor", "visitor"},
		{"village", "vocalist"},
		{"virus", "voyager"},
		{"Vulcan", "warranty"},
		{"waffle", "Waterloo"},
		{"wallet", "whimsical"},
		{"watchword", "Wichita"},
		{"wayside", "Wilmington"},
		{"willow", "Wyoming"},
		{"woodlark", "yesteryear"},
		{"Zulu", "Yucatán"}
	};
	
	public static String toHex(String arg) {
		return String.format("%x", new BigInteger(1, arg.getBytes()));
	}
	
	public static String getHash(String input) {
		if (input == null || input.equals("") || input.isEmpty()) {
			return "";
		}
		
		try {
			MessageDigest md = MessageDigest.getInstance("SHA-256");
			byte[] messageDigest = md.digest(input.getBytes());
			StringBuffer sb = new StringBuffer();
			for (int i = 0; i < messageDigest.length; i++) {
				sb.append(Integer.toString((messageDigest[i] & 0xff) + 0x100, 16).substring(1));
			}
			
			return sb.toString();
		} catch (NoSuchAlgorithmException e) {
			e.printStackTrace();
			return "";
		}
	}
	
	public static String hashToReadableString(String hash) {
		String readableString = "";
		for (int i = 0; i < hash.length(); i++) {
			readableString += hash.charAt(i);
			if (i != hash.length()-1 && i%2 == 1) {
				readableString += " ";
			}
		}
		
		return readableString;
	}
	
	public static void deleteKey(Context context, SingletonEndpoint endpoint) {
		Intent i = new Intent(context, DaemonService.class);
		i.setAction(DaemonService.ACTION_REMOVE_KEY);
		i.putExtra(de.tubs.ibr.dtn.Intent.EXTRA_ENDPOINT, (Parcelable)endpoint);
		context.startService(i);
	}
	
	public static Bitmap createIdenticon(String data) {
		try {
			MessageDigest dig = MessageDigest.getInstance("MD5");
			dig.update(data.getBytes());
			return createIdenticon(dig.digest());
		} catch (NoSuchAlgorithmException e) {
			return null;
		}
	}
	
	public static Bitmap createIdenticon(byte[] hash) {
		int background = Color.parseColor("#00000000");
		return createIdenticon(hash, background);
	}
		
	public static Bitmap createIdenticon(byte[] hash, int background) {
		Bitmap identicon = Bitmap.createBitmap(5, 5, Bitmap.Config.ARGB_8888);
		
		float[] hsv = {Integer.valueOf(hash[3]+128).floatValue()*360.0f/255.0f, 0.8f, 0.8f};
		int foreground = Color.HSVToColor(hsv);

		for (int x = 0; x < 5; x++) {
			//Enforce horizontal symmetry
			int i = x < 3 ? x : 4 - x;
			for (int y=0; y < 5; y++) {
				int pixelColor;
				//toggle pixels based on bit being on/off
				if ((hash[i] >> y & 1) == 1)
					pixelColor = foreground;
				else
					pixelColor = background;
				identicon.setPixel(x, y, pixelColor);
			}
		}
		
		Bitmap bmpWithBorder = Bitmap.createBitmap(48, 48, identicon.getConfig());
		Canvas canvas = new Canvas(bmpWithBorder);
		canvas.drawColor(background);
		identicon = Bitmap.createScaledBitmap(identicon, 46, 46, false);
		canvas.drawBitmap(identicon, 1, 1, null);

		return bmpWithBorder;
	}
}
