# MCP2515 Driver

This repository contains a two-part driver for the MCP2515
CAN controller: a interface driver and base implementations
for Linux and Cosa. Apps that use this driver need to install
`mcp2515-driver` and one of `mcp2515-linux` or `mcp2515-cosa`.

The platform-specific packages implement an interface for the
base SPI functions needed by the front-end driver.


## Example Usage

For use on linux, install `mcp2515-driver` and `mcp2515-linux`.

```c++
#include <sys/mcp2515.h>
#include <MCP2515.h>

using namespace wlp;

int main(void) {
    linux::MCP2515 base("/dev/spidev0.0", 10000000);
    MCP2515 bus(&base); // pass base to front-end
    base.begin();       // initialize SPI
    bus.begin(CAN_500KBPS, MCP_8MHz);
    // ...
}
```

For use with Cosa, install `mcp2515-cosa` instead.

```c++
#include <Cosa/MCP2515.h>
#include <MCP2515.h>

using namespace wlp;

static cosa::MCP2515 base;
static MCP2515 bus(&base);

void setup() {
    bus.begin(CAN_500KBPS, MCP_8MHz);
}

void loop() {
    // ...
}

```

## Sample Applications

This repo contains `app-cosa` and `app-linux` which each
have a `sender` and `receiver` example.

## Caveats

If you are using an 8 MHz CAN chip you must ensure you
use the 8 MHz configuration or else the chip will not
function correctly. Keep in mind that the available
bitrates for 8 MHz is lower than for 16 MHz.

Additionally, using the 8 MHz on a 16 MHz chip is "possible"
but it will cause 8 MHz chips on the network to be flooded out.
