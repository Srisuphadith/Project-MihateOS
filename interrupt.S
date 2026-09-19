.global irq1_stub
.type irq1_stub, @function

irq1_stub:

    pusha

    call keyboard_handler

    popa

    iret