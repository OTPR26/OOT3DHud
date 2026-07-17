// Print code references and containing functions for supplied addresses.

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;

public class ReferencesTo extends GhidraScript {
    @Override
    protected void run() throws Exception {
        for (String argument : getScriptArgs()) {
            Address target = toAddr(argument);
            println("TARGET " + target);
            ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(target);
            while (refs.hasNext()) {
                Reference ref = refs.next();
                Address from = ref.getFromAddress();
                Function function = currentProgram.getFunctionManager().getFunctionContaining(from);
                println(String.format("  REF %s %s %s", from, ref.getReferenceType(),
                    function == null ? "<no function>" : function.getEntryPoint().toString()));
            }
        }
    }
}
