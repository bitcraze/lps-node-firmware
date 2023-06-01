---
title: Configure TX power
page_id: configure-tx-power
---

The transmit power of the radio determines how far it reaches. The TX power can be configured, but note that there
might be rules and regulations in different countries that limits which settings that are allowed.

## How to configure

The configuration of the anchors determines the TX power of the anchors, if you are using the TWR mode you also have
to configure the [TX power of the Loco deck on the Crazyflie](https://www.bitcraze.io/documentation/repository/crazyflie-firmware/master/userguides/configuration/loco_tx_power)
to increase the system range.

The anchors must be configured with the power mode: `ForceTxPower` and the desired power level (9 is highest),
[see the low level configuration section](/docs/development/anchor-low-level-config)

The key sequence to use in the configuration is:
1. `p` - the power menu
2. `f` - for `ForceTxPower`
3. a number key for the desired power level

Remember to reboot the node for the changes to be active.
