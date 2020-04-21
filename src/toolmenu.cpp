/*
 * Copyright 2019 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "toolmenu.h"

#include <QDebug>

using namespace adiscope;

const QStringList ToolMenu::d_availableTools = QStringList() << "Oscilloscope"
							     << "Spectrum Analyzer"
							     << "Network Analyzer"
							     << "Signal Generator"
							     << "Logic Analyzer"
							     << "Pattern Generator"
							     << "Digital IO"
							     << "Voltmeter"
							     << "Power Supply"
							     << "Debugger"
							     << "Calibration";

const QStringList ToolMenu::d_availableIcons = QStringList() << ":/menu/oscilloscope.png"
							     << ":/menu/spectrum_analyzer.png"
							     << ":/menu/network_analyzer.png"
							     << ":/menu/signal_generator.png"
							     << ":/menu/logic_analyzer.png"
							     << ":/menu/pattern_generator.png"
							     << ":/menu/io.png"
							     << ":/menu/voltmeter.png"
							     << ":/menu/power_supply.png"
							     << ":/menu/debugger.png"
							     << ":/menu/calibration.png";

ToolMenu::ToolMenu(Preferences *preferences, QWidget *parent)
	: BaseMenu(parent), d_current_hw_name(""), d_buttonGroup(nullptr), d_preferences(preferences) {
	_loadState();

	d_buttonGroup = new QButtonGroup(this);
	_buildAllAvailableTools();

	connect(this, &ToolMenu::itemMovedFromTo, this, &ToolMenu::_updateToolList);

	connect(preferences, &Preferences::notify, this, &ToolMenu::_readPreferences);

	_readPreferences();
}

ToolMenu::~ToolMenu() { _saveState(); }

void ToolMenu::loadToolsFromFilter(Filter *filter) {

	if (!filter) {
		for (int i = 0; i < d_tools.size(); ++i) {
			removeMenuItem(d_tools[i].first);
		}
		d_current_hw_name = "";
		return;
	}

	if (d_current_hw_name == filter->hw_name()) {
		return;
	}

	d_current_hw_name = filter->hw_name();

	d_compatibleTools.clear();

	QVector<BaseMenuItem *> notCompatibleTools;
	QVector<int> compatiblePositions;
	for (int i = 0; i < d_tools.size(); ++i) {
		if (filter->compatible(d_tools[i].second)) {
			d_compatibleTools.push_back(d_tools[i].first);
			compatiblePositions.push_back(i);
		} else {
			notCompatibleTools.push_back(d_tools[i].first);
		}
	}
	if (notCompatibleTools.size() && d_items) {
		removeMenuItem(notCompatibleTools);
	} else {
		insertMenuItem(d_compatibleTools, compatiblePositions);
	}
}

ToolMenuItem *ToolMenu::getToolMenuItemFor(enum tool tool) {
	for (int i = 0; i < d_tools.size(); ++i) {
		if (d_tools[i].second == tool) {
			return static_cast<ToolMenuItem *>(d_tools[i].first);
		}
	}
	return nullptr;
}

QButtonGroup *ToolMenu::getButtonGroup() { return d_buttonGroup; }

void ToolMenu::_updateToolList(short from, short to) {
	if (d_items == d_tools.size()) {
		auto toMove = d_tools[from];
		d_tools.remove(from);
		d_tools.insert(to, toMove);
	}
}

void ToolMenu::_buildAllAvailableTools() {
	if (d_positions.empty()) {
		for (int i = 0; i < d_availableTools.size(); ++i) {
			d_positions.push_back(i);
		}
	} else {
		while ((d_positions.size() < d_availableTools.size())) {
			d_positions.push_back(d_positions.size());
		}
	}

	for (int i = 0; i < d_availableTools.size(); ++i) {
		ToolMenuItem *item =
			new ToolMenuItem(d_availableTools[d_positions[i]], d_availableIcons[d_positions[i]], this);
		connect(item->getToolBtn(), &QPushButton::clicked, [=]() {
			if (item->isDetached()) {
				item->detach();
			} else {
				Q_EMIT toolSelected(static_cast<tool>(d_positions[i]));
			}
		});
		connect(item, &ToolMenuItem::toggleButtonGroup, [=](bool detached) {
			if (detached) {
				d_buttonGroup->removeButton(item->getToolBtn());
			} else {
				d_buttonGroup->addButton(item->getToolBtn());
			}
		});
		connect(item, &ToolMenuItem::enableInfoWidget, this, &ToolMenu::enableInfoWidget);
		d_buttonGroup->addButton(item->getToolBtn());
		d_tools.push_back(QPair<BaseMenuItem *, tool>(item, static_cast<tool>(d_positions[i])));
		d_tools[i].first->setVisible(false);
	}
}

void ToolMenu::_saveState() {
	QSettings settings;

	settings.beginWriteArray("toolMenu/pos");
	for (int i = 0; i < d_tools.size(); ++i) {
		settings.setArrayIndex(i);
		settings.setValue("idx", QVariant(d_tools[i].second));
	}
	settings.endArray();
}

void ToolMenu::_loadState() {
	QSettings settings;

	int n = settings.beginReadArray("toolMenu/pos");
	for (int i = 0; i < n; ++i) {
		settings.setArrayIndex(i);
		d_positions.push_back(settings.value("idx").value<int>());
	}
	settings.endArray();
}

void ToolMenu::_readPreferences() {
	for (int i = 0; i < d_tools.size(); ++i) {
		ToolMenuItem *item = static_cast<ToolMenuItem *>(d_tools[i].first);
		item->enableDoubleClickToDetach(d_preferences->getDouble_click_to_detach());
	}
}
