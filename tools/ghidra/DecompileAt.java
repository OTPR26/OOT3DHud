// Decompile one or more function addresses supplied as script arguments.

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileAt extends GhidraScript {
    @Override
    protected void run() throws Exception {
        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);

        for (String argument : getScriptArgs()) {
            Address address = toAddr(argument);
            Function function = currentProgram.getFunctionManager().getFunctionContaining(address);
            if (function == null) {
                println("NO_FUNCTION " + address);
                continue;
            }

            println("FUNCTION " + function.getEntryPoint());
            DecompileResults result = decompiler.decompileFunction(function, 60, monitor);
            if (!result.decompileCompleted()) {
                println("DECOMPILE_FAILED " + result.getErrorMessage());
                continue;
            }
            println(result.getDecompiledFunction().getC());
        }

        decompiler.dispose();
    }
}
