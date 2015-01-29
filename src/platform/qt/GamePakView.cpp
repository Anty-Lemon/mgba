/* Copyright (c) 2013-2014 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "GamePakView.h"

#include "ConfigController.h"
#include "GameController.h"

extern "C" {
#include "gba-thread.h"
}

using namespace QGBA;

GamePakView::GamePakView(GameController* controller, ConfigController* config, QWidget* parent)
	: QWidget(parent)
	, m_controller(controller)
	, m_config(config)
{
	m_ui.setupUi(this);

	connect(controller, SIGNAL(gameStarted(GBAThread*)), this, SLOT(gameStarted(GBAThread*)));
	connect(controller, SIGNAL(gameStopped(GBAThread*)), this, SLOT(gameStopped()));

	connect(m_ui.hwAutodetect, &QAbstractButton::toggled, [this] (bool enabled) {
		m_ui.hwRTC->setEnabled(!enabled);
		m_ui.hwGyro->setEnabled(!enabled);
		m_ui.hwLight->setEnabled(!enabled);
		m_ui.hwTilt->setEnabled(!enabled);
		m_ui.hwRumble->setEnabled(!enabled);
	});

	connect(m_ui.savetype, SIGNAL(currentIndexChanged(int)), this, SLOT(updateOverrides()));
	connect(m_ui.hwAutodetect, SIGNAL(clicked()), this, SLOT(updateOverrides()));
	connect(m_ui.hwRTC, SIGNAL(clicked()), this, SLOT(updateOverrides()));
	connect(m_ui.hwGyro, SIGNAL(clicked()), this, SLOT(updateOverrides()));
	connect(m_ui.hwLight, SIGNAL(clicked()), this, SLOT(updateOverrides()));
	connect(m_ui.hwTilt, SIGNAL(clicked()), this, SLOT(updateOverrides()));
	connect(m_ui.hwRumble, SIGNAL(clicked()), this, SLOT(updateOverrides()));

	connect(m_ui.save, SIGNAL(clicked()), this, SLOT(saveOverride()));

	if (controller->isLoaded()) {
		gameStarted(controller->thread());
	}
}

void GamePakView::saveOverride() {
	if (!m_config) {
		return;
	}
	m_config->saveOverride(m_override);
}

void GamePakView::updateOverrides() {
	m_override = (GBACartridgeOverride) {
		"",
		static_cast<SavedataType>(m_ui.savetype->currentIndex() - 1),
		GPIO_NO_OVERRIDE,
		0xFFFFFFFF
	};

	if (!m_ui.hwAutodetect->isChecked()) {
		m_override.hardware = GPIO_NONE;
		if (m_ui.hwRTC->isChecked()) {
			m_override.hardware |= GPIO_RTC;
		}
		if (m_ui.hwGyro->isChecked()) {
			m_override.hardware |= GPIO_GYRO;
		}
		if (m_ui.hwLight->isChecked()) {
			m_override.hardware |= GPIO_LIGHT_SENSOR;
		}
		if (m_ui.hwTilt->isChecked()) {
			m_override.hardware |= GPIO_TILT;
		}
		if (m_ui.hwRumble->isChecked()) {
			m_override.hardware |= GPIO_RUMBLE;
		}
	}

	if (m_override.savetype != SAVEDATA_AUTODETECT || m_override.hardware != GPIO_NO_OVERRIDE) {
		m_controller->setOverride(m_override);
	} else {
		m_controller->clearOverride();
	}
}

void GamePakView::gameStarted(GBAThread* thread) {
	if (!thread->gba) {
		gameStopped();
		return;
	}
	m_ui.savetype->setCurrentIndex(thread->gba->memory.savedata.type + 1);
	m_ui.savetype->setEnabled(false);

	m_ui.hwAutodetect->setEnabled(false);
	m_ui.hwRTC->setEnabled(false);
	m_ui.hwGyro->setEnabled(false);
	m_ui.hwLight->setEnabled(false);
	m_ui.hwTilt->setEnabled(false);
	m_ui.hwRumble->setEnabled(false);

	m_ui.hwRTC->setChecked(thread->gba->memory.gpio.gpioDevices & GPIO_RTC);
	m_ui.hwGyro->setChecked(thread->gba->memory.gpio.gpioDevices & GPIO_GYRO);
	m_ui.hwLight->setChecked(thread->gba->memory.gpio.gpioDevices & GPIO_LIGHT_SENSOR);
	m_ui.hwTilt->setChecked(thread->gba->memory.gpio.gpioDevices & GPIO_TILT);
	m_ui.hwRumble->setChecked(thread->gba->memory.gpio.gpioDevices & GPIO_RUMBLE);

	GBAGetGameCode(thread->gba, m_override.id);
	m_override.hardware = thread->gba->memory.gpio.gpioDevices;
	m_override.savetype = thread->gba->memory.savedata.type;

	m_ui.save->setEnabled(m_config);
}

void GamePakView::gameStopped() {	
	m_ui.savetype->setCurrentIndex(0);
	m_ui.savetype->setEnabled(true);

	m_ui.hwAutodetect->setEnabled(true);
	m_ui.hwRTC->setEnabled(!m_ui.hwAutodetect->isChecked());
	m_ui.hwGyro->setEnabled(!m_ui.hwAutodetect->isChecked());
	m_ui.hwLight->setEnabled(!m_ui.hwAutodetect->isChecked());
	m_ui.hwTilt->setEnabled(!m_ui.hwAutodetect->isChecked());
	m_ui.hwRumble->setEnabled(!m_ui.hwAutodetect->isChecked());

	m_ui.hwRTC->setChecked(false);
	m_ui.hwGyro->setChecked(false);
	m_ui.hwLight->setChecked(false);
	m_ui.hwTilt->setChecked(false);
	m_ui.hwRumble->setChecked(false);

	m_ui.save->setEnabled(false);
}
