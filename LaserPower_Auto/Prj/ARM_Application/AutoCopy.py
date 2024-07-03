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
    imageName = "ARM_App"
    timeStamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
    srcName = '.\\ARM_Application.bin'
    dstName = '..\\..\\Bin\\V0.1\\ARM_App\\' + imageName + '_' + timeStamp + '.bin'
    shutil.copy(srcName, dstName)
    dstName = '..\\..\\Tool\\Burn\\Bin\\' + 'App.bin'
    shutil.copy(srcName, dstName)
    dstName = '..\\..\\Tool\\Boot\\UartBootTool\\' + 'App.bin'
    shutil.copy(srcName, dstName) 