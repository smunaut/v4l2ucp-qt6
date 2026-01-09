/*  v4l2ucp - A universal control panel for all V4L2 devices
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

#include "previewSettings.h"
#include <QMessageBox>
#include <QSettings>

PreviewSettingsDialog::PreviewSettingsDialog(QWidget *parent) : QDialog(parent) {
    ui.setupUi(this);

    connectSignals();
    loadSettings();
}

void PreviewSettingsDialog::connectSignals() {
    QObject::connect(ui.envList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this,
                     SLOT(envItemDoubleClicked(QListWidgetItem *)));
    QObject::connect(ui.envList, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this,
                     SLOT(listCurItemChanged(QListWidgetItem *, QListWidgetItem *)));
    QObject::connect(ui.argList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this,
                     SLOT(argItemDoubleClicked(QListWidgetItem *)));
    QObject::connect(ui.argList, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this,
                     SLOT(listCurItemChanged(QListWidgetItem *, QListWidgetItem *)));
    QObject::connect(ui.addEnvBut, SIGNAL(clicked()), this, SLOT(addEnvItemClicked()));
    QObject::connect(ui.addArgBut, SIGNAL(clicked()), this, SLOT(addArgItemClicked()));
    QObject::connect(ui.removeEnvBut, SIGNAL(clicked()), this, SLOT(delEnvItemClicked()));
    QObject::connect(ui.removeArgBut, SIGNAL(clicked()), this, SLOT(delArgItemClicked()));
    QObject::connect(ui.defaultsBut, SIGNAL(clicked()), this, SLOT(defaultsClicked()));
}

void PreviewSettingsDialog::loadSettings() {
    QSettings settings(APP_ORG, APP_NAME);

    if (settings.contains(SETTINGS_APP_BINARY_NAME)) {
        ui.appNameEdit->setText(settings.value(SETTINGS_APP_BINARY_NAME).toString());
    } else {
        ui.appNameEdit->setText("mplayer");
    }

    if (settings.contains(SETTINGS_ENV_LIST)) {
        QList<QVariant> envList = settings.value(SETTINGS_ENV_LIST).toList();
        QList<QVariant>::iterator begin, end;
        for (begin = envList.begin(), end = envList.end(); begin != end; begin++) {
            new QListWidgetItem((*begin).toString(), ui.envList);
        }
    } else {
        new QListWidgetItem("LD_PRELOAD=/usr/lib/libv4l/v4l2convert.so", ui.envList);
    }

    if (settings.contains(SETTINGS_ARG_LIST)) {
        QList<QVariant> argList = settings.value(SETTINGS_ARG_LIST).toList();
        QList<QVariant>::iterator begin, end;
        for (begin = argList.begin(), end = argList.end(); begin != end; begin++) {
            new QListWidgetItem((*begin).toString(), ui.argList);
        }
    } else {
        new QListWidgetItem("tv://", ui.argList);
    }
}

PreviewSettingsDialog::~PreviewSettingsDialog() {}

void PreviewSettingsDialog::saveSettings() {
    QSettings settings(APP_ORG, APP_NAME);

    settings.setValue(SETTINGS_APP_BINARY_NAME, ui.appNameEdit->text());
    QList<QVariant> varList;
    QList<QVariant>::iterator begin, end;
    QListWidgetItem *item;
    for (int i = 0; i < ui.envList->count(); i++) {
        item = ui.envList->item(i);
        if (item) {
            varList << QVariant(item->text());
        }
    }
    settings.setValue(SETTINGS_ENV_LIST, varList);
    varList.clear();
    for (int i = 0; i < ui.argList->count(); i++) {
        item = ui.argList->item(i);
        if (item) {
            varList << QVariant(item->text());
        }
    }
    settings.setValue(SETTINGS_ARG_LIST, varList);

    settings.sync();
}

void PreviewSettingsDialog::envItemDoubleClicked(QListWidgetItem *item) { ui.envList->openPersistentEditor(item); }

void PreviewSettingsDialog::argItemDoubleClicked(QListWidgetItem *item) { ui.argList->openPersistentEditor(item); }

void PreviewSettingsDialog::addEnvItemClicked() {
    if (!ui.envEdit->text().isEmpty()) {
        new QListWidgetItem(ui.envEdit->text(), ui.envList);
        ui.envEdit->clear();
    } else {
        QMessageBox::warning(NULL, "v4l2ucp", "Empty values seems to be meaningless!");
    }
}

void PreviewSettingsDialog::addArgItemClicked() {
    if (!ui.argEdit->text().isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem(ui.argEdit->text(), ui.argList);
        ui.argEdit->clear();
    } else {
        QMessageBox::warning(NULL, "v4l2ucp", "Empty values seems to be meaningless!");
    }
}
void PreviewSettingsDialog::listCurItemChanged(QListWidgetItem *newItem, QListWidgetItem *oldItem) {
    if (oldItem) {
        oldItem->listWidget()->closePersistentEditor(oldItem);
    }
}

void PreviewSettingsDialog::delEnvItemClicked() {
    if (ui.envList->currentItem()) {
        delete ui.envList->currentItem();
    } else {
        QMessageBox::warning(NULL, "v4l2ucp", "Select entry to delete.");
    }
}

void PreviewSettingsDialog::delArgItemClicked() {
    if (ui.argList->currentItem()) {
        delete ui.argList->currentItem();
    } else {
        QMessageBox::warning(NULL, "v4l2ucp", "Select entry to delete.");
    }
}

void PreviewSettingsDialog::defaultsClicked() {
    ui.argList->clear();
    ui.envList->clear();
    ui.argEdit->clear();
    ui.envEdit->clear();
    ui.appNameEdit->setText("mplayer");

    new QListWidgetItem("LD_PRELOAD=/usr/lib/libv4l/v4l2convert.so", ui.envList);
    new QListWidgetItem("tv://", ui.argList);
}
