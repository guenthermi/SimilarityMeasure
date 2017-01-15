
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.NavigableMap;
import java.util.TreeMap;

import org.mapdb.BTreeMap;
import org.mapdb.DB;
import org.mapdb.DBMaker;
import org.wikidata.wdtk.datamodel.interfaces.EntityDocumentProcessor;
import org.wikidata.wdtk.datamodel.interfaces.ItemDocument;
import org.wikidata.wdtk.datamodel.interfaces.ItemIdValue;
import org.wikidata.wdtk.datamodel.interfaces.PropertyDocument;
import org.wikidata.wdtk.datamodel.interfaces.Statement;
import org.wikidata.wdtk.datamodel.interfaces.ValueSnak;
import org.wikidata.wdtk.dumpfiles.DumpContentType;
import org.wikidata.wdtk.dumpfiles.DumpProcessingController;
import org.wikidata.wdtk.dumpfiles.MwDumpFile;

public class Main implements EntityDocumentProcessor {

	final static boolean ONLY_CURRENT_REVISIONS = true;
	final static boolean OFFLINE_MODE = false;
	final static int TIMEOUT_SEC = 0;

	/**
	 * path to the MapDB file
	 */
	public final static String DATABASE_PATH = "data/dbase";

	public final static String OUTPUT_PATH = "output/edgeIndex.txt";

	public final static String OUTPUT_PATH_PROPERTY_USAGE = "output/propertyUsage.txt";

	/**
	 * File to store maps from MapDB
	 */
	File dbFile;

	/**
	 * MapDB database
	 */
	DB db;

	/**
	 * Mapping item ids to maps that map properties (incoming and outgoing) of
	 * the item to lists of entities which are connected through this properties
	 * with the item.
	 */
	BTreeMap<Integer, NavigableMap<Integer, List<Integer>>> store;

	/**
	 * Stores the frequency of property usage.
	 */
	BTreeMap<Integer, Integer> propertyUsage;

	int count = 0;

	public static void main(String args[]) {
		Main processor = new Main();
		processor.initMaps();
		processor.processDump();
		processor.printFile();
	}

	@Override
	public void processItemDocument(ItemDocument document) {
		count++;
		if (this.count % 10000 == 0) {
			System.out.println("Processed " + count + " Entities");
		}

		Integer entryId = getIntId(document.getItemId().getId());
		NavigableMap<Integer, List<Integer>> entry;
		if (store.containsKey(entryId)) {
			entry = store.get(entryId);
		} else {
			entry = new TreeMap<>();
		}
		Iterator<Statement> it = document.getAllStatements();
		while (it.hasNext()) {
			Statement stmt = it.next();
			if (stmt.getClaim().getMainSnak() instanceof ValueSnak) {
				ValueSnak vs = (ValueSnak) stmt.getClaim().getMainSnak();
				if (vs.getValue() instanceof ItemIdValue) {
					ItemIdValue entity = (ItemIdValue) vs.getValue();
					String propertyId = vs.getPropertyId().getId();
					int propertyIntId = getIntId(propertyId);

					// property usage
					Integer usage = propertyUsage.get(propertyIntId);
					if (usage == null) {
						propertyUsage.put(propertyIntId, 1);
					} else {
						propertyUsage.put(propertyIntId, ++usage);
					}

					// outgoing properties
					List<Integer> itemList;
					if ((itemList = entry.get(propertyIntId)) == null) {
						itemList = new ArrayList<Integer>();
						entry.put(propertyIntId, itemList);
					}
					itemList.add(getIntId(entity.getId()));

				}
			}

		}
		store.put(entryId, entry);

	}

	@Override
	public void processPropertyDocument(PropertyDocument document) {
		count++;
		if (this.count % 10000 == 0) {
			System.out.println("Processed " + count + " Entities");
		}

	}

	public void initMaps() {
		dbFile = new File(DATABASE_PATH);
		if (dbFile.exists()) {
			dbFile.delete();
		}
		try {
			dbFile.createNewFile();
		} catch (IOException e) {
			System.err.println(e.getMessage());
		}
		db = DBMaker.fileDB(dbFile).transactionDisable()
				.asyncWriteFlushDelay(100).compressionEnable().make();

		store = db.treeMap("store");

		propertyUsage = db.treeMap("propertyUsage");

	}

	public void printFile() {
		File file = new File(OUTPUT_PATH);
		File file2 = new File(OUTPUT_PATH_PROPERTY_USAGE);
		if (file.exists()) {
			file.delete();
		}
		if (file2.exists()){
			file2.delete();
		}
		try {
			file.createNewFile();
			file2.createNewFile();
		} catch (IOException e) {
			System.err.println(e.getMessage());
		}
		FileOutputStream ostream = null;
		FileOutputStream propertyUsageStream = null;
		try {
			ostream = new FileOutputStream(file);
			BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(
					ostream));

			propertyUsageStream = new FileOutputStream(file2);
			BufferedWriter propertyWriter = new BufferedWriter(new OutputStreamWriter(propertyUsageStream));
			for (Integer i : propertyUsage.descendingKeySet()){
				propertyWriter.write(i.toString() + ":" + propertyUsage.get(i) + ";");
			}
			propertyWriter.close();


			for (int itemId : store.descendingKeySet()) {
				StringBuilder builder = new StringBuilder();
				builder.append(itemId);
				builder.append(";");

				NavigableMap<Integer, List<Integer>> propertyLinks = store
						.get(itemId);
				for (int propertyId : propertyLinks.descendingKeySet()) {
					builder.append(propertyId);
					List<Integer> itemList = propertyLinks.get(propertyId);
					Collections.sort(itemList);
					for (int item2 : itemList) {
						builder.append(",");
						builder.append(item2);
					}
					builder.append(";");
				}
				writer.write(builder.toString());
				writer.newLine();

			}
			writer.close();
		} catch (IOException e) {
			System.err.println(e.getMessage());
		}
	}

	int getIntId(String rawId) {
		return Integer.decode(rawId.replace("Q", "").replace("P", ""));
	}

	/**
	 * Configures the dump processing.
	 */
	public void processDump() {

		// Controller object for processing dumps:
		DumpProcessingController dumpProcessingController = new DumpProcessingController(
				"wikidatawiki");
		dumpProcessingController.setOfflineMode(OFFLINE_MODE);

		dumpProcessingController.registerEntityDocumentProcessor(this, null,
				ONLY_CURRENT_REVISIONS);

		MwDumpFile dumpFile = null;
		// Start processing (may trigger downloads where needed):
		dumpFile = dumpProcessingController
				.getMostRecentDump(DumpContentType.JSON);
		if (dumpFile != null) {
			dumpProcessingController.processDump(dumpFile);
		}

	}

}
