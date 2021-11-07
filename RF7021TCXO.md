# RF7021SE TCXO replacement

+ Find default `19.68MHz` TCXO on the PCB:

![RF7021](img/rf7021tcxo1.jpg)

+ And unsolder it:

![RF7021](img/rf7021tcxo2.jpg)

+ Replace with `12.288Mhz` or `14.745MHz`. Default footprint is 3225, but 5032 can be placed as well:

![RF7021](img/rf7021tcxo3.jpg)

+ To use 5032 chip you may have to use VCC workaround:

![RF7021](img/rf7021tcxo4.jpg)

+ Don't forget to change the `RF_TCXO` value to match your TCXO!