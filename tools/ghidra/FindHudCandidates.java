// Locate OoT3D UI/resource strings and print the functions that reference them.
// Run with Ghidra's analyzeHeadless using -postScript FindHudCandidates.java.

import java.util.Locale;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.DataType;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;

public class FindHudCandidates extends GhidraScript {
    private static final String[] TERMS = {
        "hud", "layout", "touch", "button", "item", "heart", "magic",
        "rupee", "menu", ".qly", ".qsp", ".ctxb", "queen"
    };

    @Override
    protected void run() throws Exception {
        Listing listing = currentProgram.getListing();
        for (Data data : listing.getDefinedData(true)) {
            if (monitor.isCancelled()) {
                break;
            }

            DataType type = data.getDataType();
            Object value = data.getValue();
            if (value == null || !type.getName().toLowerCase(Locale.ROOT).contains("string")) {
                continue;
            }

            String text = value.toString();
            String lower = text.toLowerCase(Locale.ROOT);
            if (!matches(lower)) {
                continue;
            }

            Address address = data.getAddress();
            println(String.format("STRING %s %s", address, text));
            ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(address);
            while (refs.hasNext()) {
                Reference ref = refs.next();
                Address from = ref.getFromAddress();
                Function function = listing.getFunctionContaining(from);
                println(String.format("  REF %s %s", from,
                    function == null ? "<no function>" : function.getEntryPoint().toString()));
            }
        }
    }

    private static boolean matches(String text) {
        for (String term : TERMS) {
            if (text.contains(term)) {
                return true;
            }
        }
        return false;
    }
}
