import serial
import struct

# Kullanim: python pc_control.py
# Varsayilan port ve baud degistirilebilir.
PORT  = "/dev/ttyUSB0"
BAUD  = 115200

# --- Frame builder'lar ---

def linear(cmd: str) -> bytes:
    """cmd: 'idle' | 'extend' | 'retract'"""
    codes = {"idle": 0b00, "extend": 0b01, "retract": 0b10}
    c = codes[cmd.lower()]
    return bytes([(0b00 << 6) | (c << 4)])

def throttle(raw: int) -> bytes:
    """raw: 0-4095 (12-bit DAC degeri)"""
    raw = max(0, min(4095, raw))
    b0 = (0b01 << 6) | ((raw >> 6) & 0x3F)
    b1 = (raw & 0x3F) << 2
    return bytes([b0, b1])

def step(direction: int, angle: int) -> bytes:
    """direction: 0 veya 1 | angle: 0-8191 (0=dur)"""
    angle = max(0, min(8191, angle))
    direction = direction & 0x01
    b0 = (0b10 << 6) | (direction << 5) | ((angle >> 8) & 0x1F)
    b1 = angle & 0xFF
    return bytes([b0, b1])

def sr(data: int) -> bytes:
    """data: 0-255 (shift register byte).

    NOT: MCU tarafinda bu byte artik dogrudan shift register'a yazilmiyor;
    sadece SR_MASK_PC_SPARE (app_config.h) bitlerine uygulaniyor. bit0 (RC
    aux) ve bit6-7 (linear motor) baska subsystem'ler tarafindan sahiplenildigi
    icin buradan gelen degerdeki o bitler etkisiz kalir. Opcode->handler
    eslesmesi icin otoriter kaynak: Core/Src/pc_control.c icindeki s_commands[].
    """
    b0 = (0b11 << 6)
    b1 = data & 0xFF
    return bytes([b0, b1])


# --- Interaktif demo ---

def main():
    ser = serial.Serial(PORT, BAUD, timeout=1)
    print(f"Baglandi: {PORT} @ {BAUD}")
    print("Komutlar: linear / throttle / step / sr / quit")

    while True:
        try:
            cmd = input("> ").strip().lower()
        except (EOFError, KeyboardInterrupt):
            break

        if cmd == "quit":
            break

        elif cmd == "linear":
            val = input("  idle / extend / retract: ").strip()
            ser.write(linear(val))

        elif cmd == "throttle":
            val = int(input("  raw 0-4095: "))
            ser.write(throttle(val))

        elif cmd == "step":
            d = int(input("  direction 0/1: "))
            a = int(input("  angle 0-8191:  "))
            ser.write(step(d, a))

        elif cmd == "sr":
            val = int(input("  byte 0-255: "))
            ser.write(sr(val))

        else:
            print("  Bilinmeyen komut.")

    ser.close()


if __name__ == "__main__":
    main()
