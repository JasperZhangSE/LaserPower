# -*- coding: UTF8 -*-

''' Imports '''
import os
import sys
import shutil
import sys
import datetime

''' Main - Entry point '''
if __name__ == '__main__':
    if not os.path.exists('..\\..\\Bin\\V0.1\\C51_App'):
        os.makedirs('..\\..\\Bin\\V0.1\\C51_App')
    imageName = "C51_App"
    timeStamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
    srcName = '.\\list\\C51_App.hex'
    dstName = '..\\..\\Bin\\V0.1\\C51_App\\' + imageName + '_' + timeStamp + '.hex'
    shutil.copy(srcName, dstName)