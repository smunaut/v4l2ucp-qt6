/*  v4l2ucp - A universal control panel for all V4L2 devices
    Copyright (C) 2005 Scott J. Bertin (scottbertin@yahoo.com)
    Copyright (C) 2009 Vasily Khoruzhick (anarsoul@gmail.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <QApplication>

#include "mainWindow.h"

void usage(const char *argv0) {
    using std::cout;
    using std::endl;

    cout << "Usage: " << argv0 << " [-h | --help] [filename]..." << endl;
    cout << "-h or --help will print this message and exit." << endl;
    cout << "filename is one or more device files for the ";
    cout << "V4L2 devices to control." << endl;
    cout << "If no filenames are given, the filename specified in the" << endl;
    cout << "environment variable V4L2UCP_DEV, or /dev/video0 will be used.";
    cout << endl;
    cout << "Also accepts standard Qt arguments." << endl;
}

int main(int argc, char **argv) {
    MainWindow *w;
    QApplication a(argc, argv);
    bool windowOpened = false;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage(argv[0]);
            exit(EXIT_SUCCESS);
        }
        w = MainWindow::openFile(argv[i]);
        if (w) {
            w->show();
            windowOpened = true;
        }
    }

    if (argc == 1) {
        const char *fname = getenv("V4L2UCP_DEV");
        if (fname) {
            w = MainWindow::openFile(fname);
            if (w) {
                w->show();
                windowOpened = true;
            }
        } else {
            w = MainWindow::openFile("/dev/video0");
            if (w) {
                w->show();
                windowOpened = true;
            } else {
                MainWindow::fileOpen();
                windowOpened = true;
            }
        }
    }

    if (!windowOpened)
        exit(EXIT_FAILURE);

    a.connect(&a, &QGuiApplication::lastWindowClosed, &a, &QApplication::quit);
    return a.exec();
}
