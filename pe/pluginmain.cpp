// Copyright (c) 2017-2019 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "pluginmain.h"

int Plugin::handle;
HWND Plugin::hwndDlg;
int Plugin::hMenu;
int Plugin::hMenuDisasm;
int Plugin::hMenuDump;
int Plugin::hMenuStack;
QString Plugin::sCurrentFileName;
PEWidget *Plugin::pFormResult=nullptr;
FormatWidget::OPTIONS Plugin::options={};
QFile *Plugin::pFile=nullptr;

extern "C" __declspec(dllexport) bool pluginit(PLUG_INITSTRUCT* initStruct)
{
    initStruct->pluginVersion = plugin_version;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strcpy_s(initStruct->pluginName, plugin_name);

    Plugin::handle = initStruct->pluginHandle;
    QtPlugin::Init();

    Plugin::options={};
    Plugin::pFile=nullptr;
    Plugin::options.nImageBase=-1;

    return true;
}

extern "C" __declspec(dllexport) void plugsetup(PLUG_SETUPSTRUCT* setupStruct)
{
    Plugin::hwndDlg = setupStruct->hwndDlg;
    Plugin::hMenu = setupStruct->hMenu;
    Plugin::hMenuDisasm = setupStruct->hMenuDisasm;
    Plugin::hMenuDump = setupStruct->hMenuDump;
    Plugin::hMenuStack = setupStruct->hMenuStack;
    GuiExecuteOnGuiThread(QtPlugin::Setup);
    QtPlugin::WaitForSetup();

    _plugin_menuaddentry(Plugin::hMenu, 0, "&About...");
}

extern "C" __declspec(dllexport) bool plugstop()
{
    GuiExecuteOnGuiThread(QtPlugin::Stop);
    QtPlugin::WaitForStop();
    return true;
}

extern "C" __declspec(dllexport) void CBINITDEBUG(CBTYPE cbType, PLUG_CB_INITDEBUG* info)
{
    Plugin::sCurrentFileName=info->szFileName;
}

extern "C" __declspec(dllexport) void CBMENUENTRY(CBTYPE cbType, PLUG_CB_MENUENTRY* info)
{
    switch(info->hEntry)
    {
    case Plugin::PLUGIN_MENU_ABOUT:
        if(Plugin::pFormResult)
        {
            DialogInfo di(Plugin::pFormResult);

            di.exec();
        }
        break;
    default:
        break;
    }
}

extern "C" __declspec(dllexport) void CBCREATEPROCESS(CBTYPE cbType, PLUG_CB_CREATEPROCESS* info)
{
    if(Plugin::pFormResult)
    {
        if(Plugin::pFile)
        {
            if(Plugin::pFile->isOpen())
            {
                Plugin::pFile->close();
            }

            delete Plugin::pFile;
            Plugin::pFile=0;
        }

        if(Plugin::sCurrentFileName!="")
        {
            Plugin::pFile=new QFile;

            Plugin::pFile->setFileName(Plugin::sCurrentFileName);

            if(XBinary::tryToOpen(Plugin::pFile))
            {
                Plugin::pFormResult->show();
                Plugin::pFormResult->setData(Plugin::pFile,&Plugin::options);
            }
        }
    }
}

extern "C" __declspec(dllexport) void CBSTOPDEBUG(CBTYPE cbType, PLUG_CB_STOPDEBUG* info)
{
    if(Plugin::pFile)
    {
        if(Plugin::pFile->isOpen())
        {
            Plugin::pFile->close();
        }

        delete Plugin::pFile;
        Plugin::pFile=0;
    }

    Plugin::pFormResult->hide();
}

