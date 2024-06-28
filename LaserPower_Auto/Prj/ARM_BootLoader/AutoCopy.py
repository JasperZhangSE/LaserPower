# -*- coding: UTF8 -*-

''' Imports '''
import os
import sys
import shutil
import datetime

''' Main - Entry point '''
if __name__ == '__main__':
    if not os.path.exists('..\\..\\Bin\\V0.1\\ARM_App'):
        os.makedirs('..\\..\\Bin\\V0.1\\ARM_App')
    imageName = "ARM_Boot"
    timeStamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
    srcName = '.\\10-0512-001-V0.1_ARM_BootLoader.bin'
    dstName = '..\\..\\Bin\\V0.1\\ARM_Boot\\' + imageName + '_' + timeStamp + '.bin'
    shutil.copy(srcName, dstName)
    dstName = '..\\..\\Tool\\Burn\\Bin\\' + 'Boot.bin'
    shutil.copy(srcName, dstName)
