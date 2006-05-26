/* museeq - a Qt client to museekd
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "userlistitem.h"

#include <qpixmapcache.h>

#include "userlistview.h"
#include "util.h"
#include "images.h"
#include "museeq.h"
#include <iostream>

UserListItem::UserListItem(UserListView *_parent, const QString& _u, uint _s, uint _sp, uint _f, const QString& _c)
	     : QListViewItem(static_cast<QListView *>(_parent)), mUser(_u) {

	setText(1, mUser);
	setStatus(_s);
	setSpeed(_sp);
	setFiles(_f);
	setComments(_c);
	setDragEnabled(true);
}
void UserListItem::updateUserStatus() {
	setStatus(mStatus);
}
void UserListItem::setStatus(uint s) {
	mStatus = s;
	
	QString icon;
	switch(s) {
	case 0:
		if (museeq->isBanned(mUser))
			icon = "offline-banned";
		else if (museeq->isIgnored(mUser))
			icon = "offline-ignored";
		else if (museeq->isTrusted(mUser))
			icon = "offline-trusted";
		else if (museeq->isBuddy(mUser))
			icon = "offline-buddied";
		else
			icon = "offline";
		break;
	case 1:
		if (museeq->isBanned(mUser))
			icon = "away-banned";
		else if (museeq->isIgnored(mUser))
			icon = "away-ignored";
		else if (museeq->isTrusted(mUser))
			icon = "away-trusted";
		else if (museeq->isBuddy(mUser))
			icon = "away-buddied";
		else
			icon = "away";
		break;
	default:
		if (museeq->isBanned(mUser))
			icon = "online-banned";
		else if (museeq->isIgnored(mUser))
			icon = "online-ignored";
		else if (museeq->isTrusted(mUser))
			icon = "online-trusted";
		else if (museeq->isBuddy(mUser))
			icon = "online-buddied";
		else
			icon = "online";
	}
	
	setPixmap(0, IMG(icon));
}

void UserListItem::setSpeed(uint s) {
	mSpeed = s;
	setText(2, QString().sprintf("%i", mSpeed));
}

void UserListItem::setFiles(uint f) {
	mFiles = f;
	setText(3, QString().sprintf("%u", mFiles));
}

void UserListItem::setComments(const QString& _c) {
	mComments = _c;
	setText(4, mComments);
}

void UserListItem::setAll(uint st, uint sp, uint f, const QString& c) {
	setStatus(st);
	setSpeed(sp);
	setFiles(f);
	setComments(c);
}

const QPixmap* UserListItem::pixmap() const {
	return QListViewItem::pixmap(0);
}

uint UserListItem::status() const {
	return mStatus;
}

QString UserListItem::user() const {
	return mUser;
}

uint UserListItem::speed() const {
	return mSpeed;
}

uint UserListItem::files() const {
	return mFiles;
}

QString UserListItem::comments() const {
	return mComments;
}

int UserListItem::compare(QListViewItem* i, int col, bool) const {
	UserListItem* u = static_cast<UserListItem*>(i);
	
	if(col == 0)
		return Util::cmp(mStatus, u->mStatus);
	else if(col == 1)
		return mUser.localeAwareCompare(u->mUser);
	else if(col == 2)
		return Util::cmp(mSpeed, u->mSpeed);
	else if(col == 3)
		return Util::cmp(mFiles, u->mFiles);
	else
		return mComments.localeAwareCompare(u->mComments);
}