/*
 * This example shows how to catch Java Exception from JavaScript
 */
function exceptionTest() {
    try {
        throwAnException();
    } catch (E) {
        print("---------------- JavaScript Stack Trace -------------");
        print(E.stack);
        print("---------------- Java Stack Trace -------------");
        print(E.nativeStack);
    }
}

function test() {
    exceptionTest();
}

test();
