#pragma once
#include <QStringList>
#include <QItemDelegate>
#include <QPainter>

#include "AtlasXTypes.h"

namespace AtlasX
{

//============================================================================
static QStringList vecToQStringList(Vector<String> const& vec) {
	QStringList list;
	for (const auto& str : vec) {
		list.push_back(QString::fromStdString(str));
	}
	return list;
}


template <typename T>
static QStringList mapToQStringList(const HashMap<std::string, T>& inputMap) {
	QStringList list;
	for (const auto& pair : inputMap) {
		list.push_back(QString::fromStdString(pair.first));
	}
	return list;
}




}
