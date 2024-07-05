# -*- coding: UTF8 -*-

''' Imports '''
import os
import sys
import shutil
import datetime

''' Main - Entry point '''
if __name__ == '__main__':
    if not os.path.exists('..\\..\\Bin\\V0.1\\ARM_Boot'):
        os.makedirs('..\\..\\Bin\\V0.1\\ARM_Boot')
    imageName = "ARM_Boot"
    timeStamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
    srcName = '.\\ARM_Bootloader.bin'
    dstName = '..\\..\\Bin\\V0.1\\ARM_Boot\\' + imageName + '_' + timeStamp + '.bin'
    shutil.copy(srcName, dstName)
    dstName = '..\\..\\Tool\\Burn\\Bin\\' + 'Boot.bin'
    shutil.copy(srcName, dstName)
    # dstName = '..\\..\\Tool\\Boot\\UartBootTool\\' + 'Boot.bin'
    # shutil.copy(srcName, dstName) 