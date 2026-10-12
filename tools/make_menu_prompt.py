"""Supplement the existing keycap artwork with Menu (SDL Menu/Application).
Uses the port's Escape keycap, changing only its interior legend. No dependencies.
"""
from pathlib import Path
import struct

def generate(root,output=None):
    folder=(Path(output) if output else Path(root)/'textures')/'bindings'
    folder.mkdir(parents=True,exist_ok=True)
    data=(Path(root)/'textures/bindings/keyboard_escape.dds').read_bytes()
    rgba=bytearray(data[128:128+4096])
    def pixel(x,y): return ((31-y)*32+x)*4
    for y in range(10,22):
        for x in range(7,25):
            p=pixel(x,y)
            if rgba[p+3]>128: rgba[p:p+3]=bytes([245]*3)
    letters=['101111111101101','111100110100111','101111111111101','101101101101111']
    for n,letter in enumerate(letters):
        for y in range(5):
            for x in range(3):
                if letter[y*3+x]=='1':
                    p=pixel(8+n*4+x,14+y)
                    rgba[p:p+4]=bytes([45,45,45,255])
    for width in (32,64):
        header=bytearray(data[:128]);struct.pack_into('<I',header,16,width)
        struct.pack_into('<I',header,20,width*4)
        output=bytearray(width*32*4)
        for y in range(32):
            p=(y*width+(width-32)//2)*4
            output[p:p+128]=rgba[y*128:(y+1)*128]
        suffix='' if width==32 else '_64x32'
        (folder/('keyboard_menu'+suffix+'.dds')).write_bytes(header+output)

if __name__=='__main__': generate(Path(__file__).resolve().parents[1])
