
#include "FakeDispenser.h"

void FakeDispenser::dispense(const ProductId& productId)
{
    ++dispenseCalls;
    lastProduct = productId;
}
