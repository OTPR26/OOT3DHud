"""Check candidate UI routing and native bindings without touching saves."""
import re
import subprocess
import tempfile
import unittest
from pathlib import Path

from audit_eur_native_bindings import eur_address, jp_address, instruction_match

ROOT = Path(__file__).resolve().parents[1]
CANDIDATES = ROOT / "artifacts/issue-16"


class AuxiliaryUiTests(unittest.TestCase):
    def test_regional_bindings(self):
        clean = ROOT / "artifacts/unified-regional-156/clean-inputs"
        usa = (clean / "USA/code.bin").read_bytes()
        for region, translate in (("EUR", eur_address), ("JP", jp_address)):
            code = (clean / region / "code.bin").read_bytes()
            for entry, size in ((0x427CA8, 20), (0x428924, 136),
                                (0x471F40, 16), (0x477E10, 16)):
                for address in range(entry, entry + size, 4):
                    with self.subTest(region=region, address=hex(address)):
                        self.assertTrue(instruction_match(usa, code, address, translate))

    def test_routing(self):
        shim = r"""
#include <assert.h>
#include <stdint.h>
typedef unsigned char u8;
typedef void (*DrawSecondaryCallbackFn)(void*);
typedef void (*FinishTopPassFn)(void*);
static void* sHintContext;
static int nameActive, uiCalls, widgetCalls, hintCalls, queueCalls, viewportCalls;
static int lastViewport[4];
static void* expectedContext;
static void* expectedPass;
static int IsNameEntryActive(void) { return nameActive; }
static void SetScissor(int a, int b, int c, int d, int e) {
    assert(a == 0 && b == 0 && c == 0 && d == 479 && e == 399);
}
static void SetViewport(int x, int y, int w, int h) {
    lastViewport[0]=x; lastViewport[1]=y; lastViewport[2]=w; lastViewport[3]=h;
    ++viewportCalls;
}
static void DrawSecondaryUi(void) {
    assert(lastViewport[0] == 0 && lastViewport[1] == 40);
    assert(lastViewport[2] == 480 && lastViewport[3] == 320);
    ++uiCalls;
}
static void DrawSecondaryWidgets(void) { ++widgetCalls; }
static void Hint(void* context) {
    assert(context == expectedContext);
    assert(lastViewport[0] == 90 && lastViewport[1] == 0);
    assert(lastViewport[2] == 300 && lastViewport[3] == 200);
    ++hintCalls;
}
static void Queue(void* pass) { assert(pass == expectedPass); ++queueCalls; }
#define ADDR(address) ((uintptr_t)Hint)
#define DRAW_TITLE_ADDR ((uintptr_t)Queue)
"""
        main = r"""
int main(void) {
    int context, pass;
    expectedContext=&context; expectedPass=&pass;
    assert(!DrawAuxiliaryUi(&pass) && viewportCalls == 0);
    nameActive=1;
    assert(DrawAuxiliaryUi(&pass) && uiCalls == 1 && widgetCalls == 1);
    sHintContext=&context;
    assert(DrawAuxiliaryUi(&pass) && !sHintContext);
    assert(hintCalls == 1 && queueCalls == 1 && uiCalls == 1);
    assert(lastViewport[0] == 0 && lastViewport[1] == 0);
    assert(lastViewport[2] == 480 && lastViewport[3] == 400);
    nameActive=0;
    assert(!DrawAuxiliaryUi(&pass) && hintCalls == 1);
    return 0;
}
"""
        with tempfile.TemporaryDirectory(prefix="reframed-ui-test-") as temp:
            for region in ("USA", "EUR", "JP"):
                source = (CANDIDATES / region / "source/src/single_screen.c").read_text()
                body = re.search(r"static u8 DrawAuxiliaryUi\(void\* pass\) \{.*?\n\}",
                                 source, re.S).group()
                executable = str(Path(temp) / region)
                subprocess.run(["cc", "-Wall", "-Wextra", "-Werror",
                                f"-DVersion_{region}", "-x", "c", "-", "-o", executable],
                               input=shim + body + main, text=True, check=True)
                subprocess.run([executable], check=True)


if __name__ == "__main__":
    unittest.main()
