# -*- coding: latin-1 -*-

import os

# ��ȡ�ű�����Ŀ¼�ľ���·��
script_dir = os.path.dirname(os.path.abspath(__file__))

# ���ù���Ŀ¼Ϊ�ű�����Ŀ¼
os.chdir(script_dir)

boot_name = "Boot.bin"
image_name = "App.bin"
output_name = "LaserPower.bin"

fB = open(boot_name, "rb")
fI = open(image_name, "rb")
fO = open(output_name, "wb")

if os.path.getsize(boot_name) > 65536:
    print("Error: boot image size is greater than 64KB")
    exit(0)

fO.write(fB.read(os.path.getsize(boot_name)))
fO.write(bytearray(65536 - os.path.getsize(boot_name)))
fO.write(fI.read(os.path.getsize(image_name)))
