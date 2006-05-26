/* museekd - The Museek daemon
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

#ifndef __IFACE_MESSAGES_HH__
#define __IFACE_MESSAGES_HH__

#include <Muhelp/DirEntry.hh>
#include <Museek/Messages.hh>
#include <Museek/TransferManager.hh>
#include <Museek/PeerManager.hh>
#include <Mucipher/mucipher.h>

class IfaceMessage : public NetworkMessage {
public:
	void pack(const std::string& s) { NetworkMessage::pack(s); }
	void pack(uint32 i) { NetworkMessage::pack(i); }
	void pack(unsigned char c) { NetworkMessage::pack(c); }
	void pack(off_t o) { NetworkMessage::pack(o); }
	
	inline void pack(const UserData& d) {
		pack(d.status);
		pack(d.avgspeed);
		pack(d.downloadnum);
		pack(d.files);
		pack(d.dirs);
		pack((unsigned char)d.slotsfull);
	}

	inline void pack(const struct FileEntry& fe) {
		pack(fe.size);
		pack(fe.ext);
		pack((uint32)fe.attrs.size());
		std::vector<uint32>::const_iterator ait = fe.attrs.begin();
		for(; ait != fe.attrs.end(); ++ait)
			pack(*ait);
	}
	
	inline void pack(const Transfer* tr) {
		unsigned char d = (tr->direction() == Transfer::Upload) ? 1 : 0;
		pack(d);
		pack(tr->peer()->user());
		pack(tr->path_utf8());
		pack(tr->place_in_queue());
		pack((uint32)tr->state());
		pack(tr->error());
		pack(tr->pos());
		pack(tr->size());
		pack((uint32)tr->rate());
	}
	
	inline UserData unpack_userdata() {
		UserData r;
		r.status = unpack_int();
		r.avgspeed = unpack_int();
		r.downloadnum = unpack_int();
		r.files = unpack_int();
		r.dirs = unpack_int();
		r.slotsfull = unpack_char();
		return r;
	}
	
	inline void cipher(CipherContext* ctx, const std::string& s) {
		pack((uint32)s.size());
		
		uint32 len = CIPHER_BLOCK(s.size());
		unsigned char ciph[len];
		blockCipher(ctx, (unsigned char*)s.data(), s.size(), ciph);
		
		for(uint i = 0; i < len; i++)
			pack(ciph[i]);
	}
	
	inline std::string decipher(CipherContext* ctx) {
		uint32 s_len = unpack_int(),
		       c_len = CIPHER_BLOCK(s_len);
		
		unsigned char ciph[c_len], deciph[c_len];
		for(uint32 i = 0; i < c_len; i++)
			ciph[i] = unpack_char();
		blockDecipher(ctx, ciph, c_len, deciph);
		
		return std::string((char*)deciph, s_len);
	}
	
};

#define IFACEMESSAGE(mtype, m_id) NETWORKMESSAGE(IfaceMessage, mtype, m_id)

IFACEMESSAGE(IPing, 0x0000)
/*
	Ping -- Ask the daemon to respond
	
	uint id -- Unique identifier
	
	uint id -- Unique identifier (same as what was sent)
*/
	IPing(uint32 _id = 0) : id(_id) { }
	
	MAKE
		pack(id);
	END_MAKE
	
	PARSE
		id = unpack_int();
	END_PARSE
	
	uint32 id;
END




// Server and administration messages

IFACEMESSAGE(IChallenge, 0x0001)
/*
	Challenge -- Daemon sends this authentication challenge when connected
	
	*not sent*
	
	uint version -- Daemon interface protocol revision
	string challenge -- The challenge. Stick interface password at the end of this and generate a hash
*/
	IChallenge(uint32 _v, const std::string& _c) : version(_v), challenge(_c) { }
	
	MAKE
		pack(version);
		pack(challenge);
	END_MAKE
	
	uint32 version;
	std::string challenge;
END

IFACEMESSAGE(ILogin, 0x0002)
/*
	Login -- Request a login to the daemon
	
	string algorithm -- Digest algorithm, one of the following:
		SHA1 -- SHA-1 algorithm
		SHA256 -- SHA256 algorithm
		MD5 -- MD5 algorithm
	string chresponse -- Challenge response (hex string version of challenge digest)
	uint mask -- Interface interest mask, bitwise OR-ed value of:
		0x01 -- Receive chat related messages
		0x02 -- Receive private messages
		0x04 -- Receive transfer messages
		0x08 -- Receive user info message
		0x10 -- Receive user shares messages
		0x20 -- Receive interest and recommendation messages
		0x40 -- Receive config messages
	
	bool ok -- Wether login was successful
	string message -- In case of failure, what was the error:
		INVHASH -- Invalid digest algorithm
		INVPASS -- Invalid password
	string challenge -- New challenge, in case the interface wishes to try again
*/
	
	ILogin() {}
	ILogin(bool _o, const std::string& _m, const std::string& _c) : msg(_m), chresponse(_c) { ok = _o; }

	MAKE
		pack((unsigned char) ok);
		pack(msg);
		pack(chresponse);
	END_MAKE

	PARSE
		algorithm = unpack_string();
		chresponse = unpack_string();
		mask = unpack_int();
	END_PARSE

	bool ok;
	uint32 mask;
	std::string algorithm, msg, chresponse;
END

IFACEMESSAGE(IServerState, 0x0003)
/*
	Server state -- Network connection status
	
	*not sent*
	
	bool connected -- Wether the daemon is connected to the soulseek network
	string username -- If connected, this contains the username
*/
	
	IServerState(bool _c, const std::string& _u) : connected(_c), username(_u) { }

	MAKE
		pack((unsigned char)connected);
		pack(username);
	END_MAKE

	bool connected;
	std::string username;
END

IFACEMESSAGE(ICheckPrivileges, 0x0004)
/*
	Check privileges -- Check how many seconds of privileges you have left
	
	*empty*
	
	uint seconds -- The number of seconds of privileges you have
*/
	
	ICheckPrivileges(uint32 _t = 0) : time_left(_t) { }
	
	MAKE
		pack(time_left);
	END_MAKE
	
	PARSE
	END_PARSE
	
	uint32 time_left;
END

IFACEMESSAGE(ISetStatus, 0x0005)
/*
	Set status -- Set away status
	
	uint status -- Set away status (0 == online, 1 == away)
	
	uint status -- Away status set (0 == online, 1 == away)
*/

	ISetStatus()  {}
	ISetStatus(uint32 _s) : status(_s) {}
	
	MAKE
		pack(status);
	END_MAKE
	
	PARSE
		status = unpack_int();
	END_PARSE
	
	uint32 status;
END

IFACEMESSAGE(IStatusMessage, 0x0010)
/*
	Status Message -- Forward messages to the clients
	
	*not sent*
	
	bool messagetype -- Message type ( 0 == Server, 1 == Peer )
	string message -- If connected, this contains the message
*/
	
	IStatusMessage(bool _t, const std::string& _m) : type(_t), message(_m) { }

	MAKE
		pack((unsigned char)type);
		pack(message);
	END_MAKE

	bool type;
	std::string message;
END



// Configuration messages

IFACEMESSAGE(IConfigState, 0x0100)
/*
	Configuration state -- Contents of the configuration database
	
	*not sent*
	
	uint ndomains -- number of domains registered
	*repeat ndomains*
		cipher domain -- domain name
		uint nkeys -- number of keys registered in this domain
		*repeat nkeys*
			cipher key -- name of this key
			cipher value -- ciphered value of this key
*/
	
	IConfigState(CipherContext* ctx, const std::map<std::string, StringMap>& _c) : context(ctx), config(_c) { }
	
	MAKE
		pack((uint32)config.size());
		std::map<std::string, StringMap>::const_iterator it = config.begin();
		for(; it != config.end(); ++it) {
			cipher(context, (*it).first);
			pack((uint32)(*it).second.size());
			StringMap::const_iterator kit = (*it).second.begin();
			for(; kit != (*it).second.end(); ++kit) {
				cipher(context, (*kit).first);
				cipher(context, (*kit).second);
			}
		}
	END_MAKE
	
	CipherContext* context;
	std::map<std::string, StringMap> config;
END

IFACEMESSAGE(IConfigSet, 0x0101)
/*
	Config set -- Make a change to the configuration database
	
	cipher domain -- Domain to make a change in
	cipher key -- Key to change
	cipher value -- Value to change it to
	
	cipher domain -- The domain a change took place in
	cipher key -- The key that was changed
	cipher value -- The new value
*/
	
	IConfigSet(CipherContext* ctx, const std::string& _d, const std::string& _k, const std::string& _v) 
	          : context(ctx), domain(_d), key(_k), value(_v) { }
	IConfigSet(CipherContext* ctx) : context(ctx) { }
	
	MAKE
		cipher(context, domain);
		cipher(context, key);
		cipher(context, value);
	END_MAKE
	
	PARSE
		domain = decipher(context);
		key = decipher(context);
		value = decipher(context);
	END_PARSE
	
	CipherContext* context;
	std::string domain, key, value;
END

IFACEMESSAGE(IConfigRemove, 0x0102)
/*
	Config remove -- Remove a key from the configuration database
	
	cipher domain -- Domain to remove a key from
	cipher key -- Key to remove
	
	cipher domain -- Domain that a key was removed from
	cipher key -- Name of the key that was removed
*/
	
	IConfigRemove(CipherContext* ctx, const std::string& _d, const std::string& _k)
	             : context(ctx), domain(_d), key(_k) { }
	IConfigRemove(CipherContext* ctx) : context(ctx) { }
	
	MAKE
		cipher(context, domain);
		cipher(context, key);
	END_MAKE
	
	PARSE
		domain = decipher(context);
		key = decipher(context);
	END_PARSE
	
	CipherContext* context;
	std::string domain, key;
END

IFACEMESSAGE(IConfigSetUserImage, 0x103)
/*
	Set user image -- Set the image that gets sent with the user info
	
	string image -- The image data
	
	*not sent*
*/
	
	IConfigSetUserImage() {}
	
	PARSE
		mData = unpack_vector();
	END_PARSE
	
	std::vector<unsigned char> mData;
END

	


// Peer messages

IFACEMESSAGE(IPeerExists, 0x0201)
/*
	Peer exists -- Check if a certain username is valid
	
	string username -- Name of the user to check
	
	string username -- Username that was checked
	bool exists -- Wether the username is valid or not
*/
	
	IPeerExists(const std::string& _u, bool _e) : user(_u), exists(_e) { }
	IPeerExists() { }
	
	MAKE
		pack(user);
		pack((unsigned char)exists);
	END_MAKE
	
	PARSE
		user = unpack_string();
	END_PARSE
	
	std::string user;
	bool exists;
END

IFACEMESSAGE(IPeerStatus, 0x0202)
/*
	Peer status -- Get a user's status
	
	string username -- Name of the user to get the status of
	
	string username -- Name of the user that changed status
	uint status -- User's current status, one of the following:
		0x00 -- Offline
		0x01 -- Away
		0x02 -- Online
*/
	
	IPeerStatus(const std::string& _u, uint32 _s) : user(_u), status(_s) { }
	IPeerStatus() { }
	
	MAKE
		pack(user);
		pack(status);
	END_MAKE
	
	PARSE
		user = unpack_string();
	END_PARSE
	
	std::string user;
	uint32 status;
END

IFACEMESSAGE(IPeerStats, 0x0203)
/*
	Peer statistics -- Get a users's statistics
	
	string username -- Name of the user to get the stats of
	
	string username -- Name of the user daemon is reporting statistics for
	uint avgspeed -- User's average speed
	uint numdownloads -- User's total download count (?)
	uint numfiles -- User's file-count
	uint numdirs -- User's directory-count
*/
	
	IPeerStats(const std::string _u, uint32 _a, uint32 _n, uint32 _f, uint32 _d) 
	  : user(_u), speed(_a), downloads(_n), files(_f), dirs(_d) { }
	IPeerStats() { }
	
	MAKE
		pack(user);
		pack(speed);
		pack(downloads);
		pack(files);
		pack(dirs);
	END_MAKE
	
	PARSE
		user = unpack_string();
	END_PARSE
	
	std::string user;
	uint32 speed, downloads, files, dirs;
END

IFACEMESSAGE(IPeerAddress, 0x0206)
/*
	Peer address -- Get a user's IP address and port
	
	string username -- User to get the IP of
	
	string username -- User we got the IP of
	string IP -- User's IP address
	uint port -- User's client port number
*/
	
	IPeerAddress() {}
	IPeerAddress(const std::string& _u, const std::string& _ip, uint32 _p) : user(_u), ip(_ip), port(_p) {}
	
	MAKE
		pack(user);
		pack(ip);
		pack(port);
	END_MAKE
	
	PARSE
		user = unpack_string();
	END_PARSE
	
	std::string user, ip;
	uint32 port;
END

IFACEMESSAGE(IUserInfo, 0x0204)
/*
	User info -- Get a user's user-info
	
	string username -- User to get the userinfo of
	
	string username -- User we got userinfo of
	string info -- User's self-description
	string picture -- User's picture
	uint uploads -- User's total upload count
	uint queuelen -- User's queue length
	bool slotfree -- Wether user has a free upload slot
*/
	
	IUserInfo() {}
	IUserInfo(const std::string& _u, const std::string& _i, const std::vector<unsigned char>& _p, uint32 _s, uint32 _q, bool _f)
                 : user(_u), info(_i), picture(_p) { uploads = _s, queue = _q, slotsfree = _f; }

	MAKE
		pack(user);
		pack(info);
		pack((uint32)picture.size());
		std::vector<unsigned char>::const_iterator it = picture.begin();
		for(; it != picture.end(); ++it)
			pack(*it);
		pack(uploads);
		pack(queue);
		pack((unsigned char)slotsfree);
	END_MAKE

	PARSE
		user = unpack_string();
	END_PARSE

	std::string user, info;
	std::vector<unsigned char> picture;
	uint32 uploads, queue;
	bool slotsfree;
END

IFACEMESSAGE(IUserShares, 0x0205)
/*
	User shares -- Get a user's shares
	
	string username -- User to get the shares of
	
	string username -- User the daemon got the shares of
	shares shares -- The shares
*/
	
	IUserShares() {}
	IUserShares(const std::string& _u, const Shares& _s)
                   : user(_u), shares(_s) {}

	MAKE
		pack(user);
		pack((uint32)shares.size());
		Shares::const_iterator dit = shares.begin();
		for(; dit != shares.end(); ++dit) {
			pack((*dit).first);
			pack((uint32)(*dit).second.size());
			Folder::const_iterator fit = (*dit).second.begin();
			for(; fit != (*dit).second.end(); ++fit) {
				pack((*fit).first);
				pack((*fit).second);
			}
		}
	END_MAKE

	PARSE
		user = unpack_string();
	END_PARSE

	std::string user;
	Shares shares;
END

IFACEMESSAGE(IGivePrivileges, 0x0207)
/*
	Give privileges -- Donate (part) of your privileges to another user
	
	string username -- Username of the person to donate privileges to
	uint days -- How many days of privileges to donate
	
	*not sent*
*/
	
	IGivePrivileges() {}
	
	PARSE
		user = unpack_string();
		days = unpack_int();
	END_PARSE
	
	std::string user;
	uint32 days;
END




// Chat messages

IFACEMESSAGE(IRoomState, 0x0300)
/*
	Room state -- List of rooms and joined rooms and their users
	
	*not sent*
	
	uint numrooms -- Number of rooms in the room list
	*repeat numrooms*
		string roomname -- Name of the room
		uint numusers -- Number of users in this room
	uint numjoined -- Number of rooms we've joined
	*repeat numjoined*
		string roomname -- Name of the room
		uint numusers -- Number of users in this room
		*repeat numusers*
			string username -- Name of the user
			userdata data -- User's statistics
		uint numtickers -- Number of tickers set for this room
		*repeat numtickers*
			string username -- Name of the ticker owner
			string message -- Contents of the ticker
*/
	
	IRoomState(const std::map<std::string, uint32>& _l, const std::map<std::string, RoomData>& _r, const std::map<std::string, Tickers>& _t)
                  : roomlist(_l), rooms(_r), tickers(_t) {}
	
	MAKE
		pack((uint32)roomlist.size());
		RoomList::const_iterator rit = roomlist.begin();
		for(; rit != roomlist.end(); ++rit) {
			pack((*rit).first);
			pack((*rit).second);
		}
		
		pack((uint32)rooms.size());
		std::map<std::string, RoomData>::const_iterator it = rooms.begin();
		for(; it != rooms.end(); ++it) {
			pack((*it).first);
			pack((uint32)(*it).second.size());
			RoomData::const_iterator rit = (*it).second.begin();
			for(; rit != (*it).second.end(); ++rit) {
				pack((*rit).first);
				pack((*rit).second);
			}
			const Tickers& tick = tickers[(*it).first];
			pack((uint32)tick.size());
			Tickers::const_iterator tit = tick.begin();
			for(; tit != tick.end(); ++tit) {
				pack((*tit).first);
				pack((*tit).second);
			}
		}
	END_MAKE
	
	RoomList roomlist;
	std::map<std::string, RoomData> rooms;
	std::map<std::string, Tickers> tickers;
END

IFACEMESSAGE(IRoomList, 0x0301)
/*
	Room list -- refresh room list
	
	*empty*
	
	uint numrooms -- Number of rooms in the room list
	*repeat numroos*
		string roomname -- Name of the room
		uint numusers -- Number of users in this room
*/
	
	IRoomList() { }
	IRoomList(const RoomList& _r) : roomlist(_r) { }
	
	MAKE
		pack((uint32)roomlist.size());
		RoomList::const_iterator rit = roomlist.begin();
		for(; rit != roomlist.end(); ++rit) {
			pack((*rit).first);
			pack((*rit).second);
		}
	END_MAKE
	
	PARSE
	END_PARSE
	
	RoomList roomlist;
END

IFACEMESSAGE(IPrivateMessage, 0x0302)
/*
	Private message -- Send / receive a private message
	
	string username -- The user to send a message to
	string message -- The actual message
	
	uint direction -- Direction (0 = incoming, 1 = outgoing)
	uint timestamp -- Timestamp of when the message was received (<b>server's timezone</b>)
	string username -- The user that sent the message
	string message -- The actual message
*/
	
	IPrivateMessage() {}
	IPrivateMessage(uint32 _d, uint32 _s, const std::string& _u, const std::string& _m) : direction(_d), user(_u), msg(_m) { timestamp = _s; }
	
	MAKE
		pack(direction);
		pack(timestamp);
		pack(user);
		pack(msg);
	END_MAKE
	
	PARSE
		user = unpack_string();
		msg = unpack_string();
	END_PARSE

	uint32 direction, timestamp;
	std::string user, msg;
END

IFACEMESSAGE(IJoinRoom, 0x0303)
/*
	Join room -- Join(ed) a room
	
	string room -- The name of the room to join
	
	string room -- The name of the room we joined
	uint numusers -- Number of users in this room
	*repeat numusers*
		string username -- Name of the user
		userdata data -- User's statistics
*/
	
	IJoinRoom() {}
	IJoinRoom(const std::string& _r, const RoomData& _u) : room(_r), users(_u) {}

	MAKE
		pack(room);
		pack((uint32)users.size());
		RoomData::const_iterator rit = users.begin();
		for(; rit != users.end(); ++rit) {
			pack((*rit).first);
			pack((*rit).second);
		}
	END_MAKE

	PARSE
		room = unpack_string();
	END_PARSE

	std::string room;
	RoomData users;
END

IFACEMESSAGE(ILeaveRoom, 0x0304)
/*
	Leave room -- Leave / left a room
	
	string room -- Name of the room to leave
	
	string room -- Name of the room we left
*/
	
	ILeaveRoom() {}
	ILeaveRoom(const std::string& _r) : room(_r) {}

	MAKE
		pack(room);
	END_MAKE

	PARSE
		room = unpack_string();
	END_PARSE

	std::string room;
END

IFACEMESSAGE(IUserJoinedRoom, 0x0305)
/*
	User joined room -- A user joined a room
	
	*not sent*
	
	string room -- Room the user joined
	string username -- User that joined
	userdata data -- User's statistics
*/
	
	IUserJoinedRoom() {}
	IUserJoinedRoom(const std::string& _r, const std::string& _u, const UserData& _d)
                       : room(_r), user(_u) { userdata = _d; }

	MAKE
		pack(room);
		pack(user);
		pack(userdata);
	END_MAKE
	
	std::string room, user;
	UserData userdata;
END;

IFACEMESSAGE(IUserLeftRoom, 0x0306)
/*
	User left room -- A user left a room
	
	*not sent*
	
	string room -- Room the user left
	string username -- User that left
*/
	
	IUserLeftRoom() {}
	IUserLeftRoom(const std::string _r, const std::string& _u) : room(_r), user(_u) {}

	MAKE
		pack(room);
		pack(user);
	END_MAKE

	std::string room, user;
END

IFACEMESSAGE(ISayRoom, 0x0307)
/*
	Say in room -- Say something in a chatroom
	
	string room -- The name of the room to say someting in
	string line -- What you want to say
	
	string room -- The name of the room someone said something in
	string user -- User that said something
	string line -- What the user said
*/
	
	ISayRoom() {}
	ISayRoom(const std::string& _r, const std::string& _u, const std::string& _l)
                    : room(_r), user(_u), line(_l) {}

	MAKE
		pack(room);
		pack(user);
		pack(line);
	END_MAKE

	PARSE
		room = unpack_string();
		line = unpack_string();
	END_PARSE

	std::string room, user, line;
END

IFACEMESSAGE(IRoomTickers, 0x0308)
/*
	Room tickers -- List of tickers set for a room
	
	*not sent*
	
	string room -- Which room the tickers are reported for
	uint numtickers -- How many tickers are set
	*repeat numtickers*
		string user -- The user that this ticker belongs to
		string message -- The actual ticker message
*/

	IRoomTickers(const std::string& _r, const Tickers& _t) : room(_r), tickers(_t) {}
	
	MAKE
		pack(room);
		pack((uint32)tickers.size());
		Tickers::const_iterator it = tickers.begin();
		for(; it != tickers.end(); ++it) {
			pack((*it).first);
			pack((*it).second);
		}
	END_MAKE
	
	std::string room;
	Tickers tickers;
END

IFACEMESSAGE(IRoomTickerSet, 0x0309)
/*
	Set room ticker -- Set your room ticker / a room ticker was set
	
	string room -- The room to set the ticker in
	string message -- The actual ticker message
	
	string room -- The room a ticker was set in
	string user -- The user that set the ticker
	string message -- The actual ticker message
*/

	IRoomTickerSet() {}
	IRoomTickerSet(const std::string& _r, const std::string& _u, const std::string& _m) : room(_r), user(_u), message(_m) {}
	
	MAKE
		pack(room);
		pack(user);
		pack(message);
	END_MAKE
	
	PARSE
		room = unpack_string();
		message = unpack_string();
	END_PARSE
	
	std::string room, user, message;
END




// Search messages

IFACEMESSAGE(ISearch, 0x0401)
/*
	Search -- Start a new search
	
	uint type -- Search type (0 = global, 1 = buddies, 2 = room)
	string query -- What you want to search for
	
	string query -- The query we're delivering a ticket for
	uint ticket -- The search ticket
*/
	
	ISearch(const std::string& _q, uint32 _t) : query(_q) { ticket = _t; }
	ISearch() {}

	MAKE
		pack(query);
		pack(ticket);
	END_MAKE

	PARSE
		type = unpack_int();
		query = unpack_string();
	END_PARSE

	std::string query;
	uint32 ticket, type;
END

IFACEMESSAGE(ISearchReply, 0x0402)
/*
	Search reply -- Terminate a search, or results delivered by peers
	
	uint ticket -- The ticket of the search you wish to terminate
	
	uint ticket -- Ticket the search results are for
	string username -- User that delivered the results
	bool slotfree -- Wether the user has a free upload slot
	uint avgspeed -- User's average speed
	uint queuelen -- User's queue length
	folder results -- The actual results
*/
	ISearchReply() {}
	ISearchReply(uint32 _t, const std::string& _u, bool _f, uint32 _s, uint32 _q, const std::map<std::string, FileEntry>& _r)
                    : username(_u), results(_r) { ticket = _t, slotfree = _f, speed = _s, queue = _q; }

	MAKE
		pack(ticket);
		pack(username);
		pack((unsigned char)slotfree);
		pack(speed);
		pack(queue);
		pack((uint32)results.size());
		Folder::const_iterator it = results.begin();
		for(; it != results.end(); ++it) {
			pack((*it).first);
			pack((const struct FileEntry&)(*it).second);
		}
	END_MAKE

	PARSE
		ticket = unpack_int();
	END_PARSE

	uint32 ticket, speed, queue;
	std::string username;
	std::map<std::string, FileEntry> results;
	bool slotfree;
END


IFACEMESSAGE(IBuddySearch, 0x0403)
/*
	BuddySearch -- Start a new buddy search
	
	string query -- What you want to search for
	
	*not sent*
*/
	
	IBuddySearch() {}

	PARSE
		query = unpack_string();
	END_PARSE
	
	std::string query;
END




// Transfer messages

IFACEMESSAGE(ITransferState, 0x0500)
/*
	Transfer state -- Actual state of all transfers
	
	*not sent*
	
	uint numtransfers -- Number of transfers
	*repeat numtransfers*
		transfer entry -- The transfer entry
*/
	
	ITransferState(const std::vector<Transfer *>& _t1, const std::vector<Transfer*>& _t2) : uploads(_t1), downloads(_t2) { }
	
	MAKE
		pack((uint32)(uploads.size() + downloads.size()));
		std::vector<Transfer *>::const_iterator dit = uploads.begin();
		for(; dit != uploads.end(); ++dit)
			pack(*dit);
		for(dit = downloads.begin(); dit != downloads.end(); ++dit)
			pack(*dit);
	END_MAKE
	
	const std::vector<Transfer *> uploads, downloads;
END

IFACEMESSAGE(ITransferUpdate, 0x0501)
/*
	Transfer update -- Status update of a transfer
	
	string username -- User to update a transfer of
	string path -- Path of the transfer to update (place in queue)
	
	transfer entry -- The new state of the transfer
*/
	ITransferUpdate() { }
	ITransferUpdate(const Transfer* tr) : transfer(tr) { }
	
	MAKE
		pack(transfer);
	END_MAKE
	
	PARSE
		user = unpack_string();
		path = unpack_string();
	END_PARSE
	
	const Transfer* transfer;
	bool upload;
	std::string user, path;
END

IFACEMESSAGE(ITransferRemove, 0x0502)
/*
	Transfer remove -- Remove a transfer from the list
	
	bool upload -- Remove upload? (if false, remove download)
	string username -- User to clear a transfer from
	string path -- Path of the transfer to clear
	
	bool upload -- Was an upload removed? (if false, download was removed)
	string username -- User a transfer was removed from
	string path -- Path of the transfer that was removed
*/
	
	ITransferRemove() { }
	ITransferRemove(const Transfer* _t) : transfer(_t) { }
	
	MAKE
		unsigned char d = (transfer->direction() == Transfer::Upload) ? 1 : 0;
		pack(d);
		pack(transfer->peer()->user());
		pack(transfer->path_utf8());
	END_MAKE
	
	PARSE
		upload = unpack_char() != 0;
		user = unpack_string();
		path = unpack_string();
	END_PARSE
	
	bool upload;
	std::string user, path;
	const Transfer* transfer;
END

IFACEMESSAGE(IDownloadFile, 0x0503)
/*
	Download file -- Download a file from someone (or retry an existing transfer)
	
	string username -- Username to download a file from
	string path -- Path of the file to download
	string localpath -- Path to store the file name
	off_t size -- File's size (can be 0)
	
	*not sent*
*/
	
	IDownloadFile() : user(""), path("") {}

	PARSE
		user = unpack_string();
		path = unpack_string();
		localpath = unpack_string();
		size = unpack_off();
	END_PARSE

	std::string user, path, localpath;
	off_t size;
END

IFACEMESSAGE(IDownloadFolder, 0x0504)
/*
	Download folder -- Download a folder recursively
	
	string username -- User to download a folder from
	string folder -- Path to the folder to download
	
	*not sent*
*/
	IDownloadFolder() {}
	
	PARSE
		user = unpack_string();
		folder = unpack_string();
	END_PARSE

	std::string user, folder;
END

IFACEMESSAGE(ITransferAbort, 0x0505)
/*
	Abort transfer -- Terminate a transfer
	
	bool upload -- Abort an upload? (if false, abort download)
	string username -- User to abort the transfer from
	string path -- Path of the transfer to abort
	
	*not sent*
*/
	ITransferAbort() {}
	
	PARSE
		upload = unpack_char() != 0;
		user = unpack_string();
		path = unpack_string();
	END_PARSE
	
	bool upload;
	std::string user, path;
END

IFACEMESSAGE(IUploadFile, 0x0506)
/*
	Upload file -- Upload a file from your shares to someone
	
	string username -- Username to upload a file to
	string path -- Path of the file in your shares to upload
	
	*not sent*
*/
	
	IUploadFile() : user(""), path("") {}

	PARSE
		user = unpack_string();
		path = unpack_string();
	END_PARSE

	std::string user, path;
END
IFACEMESSAGE(IGetRecommendations, 0x0600)
/*
	Get Recommendations -- Refresh Recommendations list
	
	*empty*
	
	uint numrecommendations  --  Number of recommendations
	*repeat numrecommendations*
		string recommendation -- Name of recommend
		uint  numrecommendation -- Number of users with recommend
*/

	
	IGetRecommendations( const Recommendations& _r ) : recommendations(_r) { }

	MAKE
 		pack((uint32)recommendations.size());
		Recommendations::const_iterator it = recommendations.begin();
		for(; it != recommendations.end(); ++it) {
			pack((*it).first);
			pack((uint32)(*it).second);

		}
	
	END_MAKE
	
	PARSE
	END_PARSE

	Recommendations recommendations;

END

IFACEMESSAGE(IGetGlobalRecommendations, 0x0601)
/*
	Get Global Recommendations -- Refresh Global Recommendations list
	
	*empty*
	
	uint numrecommendations  --  Number of recommendations
	*repeat numrecommendations*
		string recommendation -- Name of recommend
		uint  numrecommendation -- Number of users with recommend
*/

	IGetGlobalRecommendations() {}
	IGetGlobalRecommendations( const Recommendations& _r ) : recommendations(_r) { }

	MAKE
 		pack((uint32)recommendations.size());
		Recommendations::const_iterator it = recommendations.begin();
		for(; it != recommendations.end(); ++it) {
			pack((*it).first);
			pack((uint32)(*it).second);

		}
	
	END_MAKE
	
	PARSE
	END_PARSE

	Recommendations recommendations;

END

IFACEMESSAGE(IGetSimilarUsers, 0x0602)
/*
	Get Similar Users -- Refresh Similar Users list
	
	*empty*
	
	uint numusers  --  Number of users
	*repeat numusers*
		string users -- Name of user
		uint  numuser -- Number of user
*/

	IGetSimilarUsers() {}
	IGetSimilarUsers( const SimilarUsers& _r ) : users(_r) { }
	MAKE
 		pack((uint32)users.size());
		SimilarUsers::const_iterator it = users.begin();
		for(; it != users.end(); ++it) {
			pack((*it).first);
			pack((uint32)(*it).second);
		}
	
	END_MAKE
	
	PARSE
	END_PARSE

	SimilarUsers users;
END

IFACEMESSAGE(IGetItemRecommendations, 0x0603)
/*
	Get Item Recommendations -- Refresh Recommendations list that matches item
	
	string -- item
	
	string -- item
	uint numrecommendations  --  Number of recommendations
	*repeat numrecommendations*
		string recommendation -- Name of recommend
		uint  numrecommendation -- 0
*/

	IGetItemRecommendations () {}
	IGetItemRecommendations( const std::string& _i, const Recommendations& _r ) : item(_i), recommendations(_r) { }
	MAKE
		pack(item);
 		pack((uint32)recommendations.size());
		Recommendations::const_iterator it = recommendations.begin();
		for(; it != recommendations.end(); ++it) {
			pack((*it).first);
			pack((uint32)(*it).second);
		}
	
	END_MAKE
	
	PARSE
		item = unpack_string();
	END_PARSE

	std::string item;
	Recommendations recommendations;
END

IFACEMESSAGE(IGetItemSimilarUsers, 0x0604)
/*
	Get Item Similar Users -- Refresh Similar Users that match item
	
	string -- item
	
	string -- item
	uint numusers  --  Number of users
	*repeat numusers*
		string users -- Name of user
		uint  numuser -- Number of user
*/

	IGetItemSimilarUsers() {}
	IGetItemSimilarUsers( const std::string& _i,  const SimilarUsers& _r ) : item(_i), users(_r) { }
	MAKE
		pack(item);
 		pack((uint32)users.size());
		SimilarUsers::const_iterator it = users.begin();
		for(; it != users.end(); ++it) {
			pack((*it).first);
			pack((uint32)(*it).second);
		}
	
	END_MAKE
	
	PARSE
		item = unpack_string();
	END_PARSE

	std::string item;
	SimilarUsers users;
END

IFACEMESSAGE(IAddInterest, 0x0610)
/*
	Add Like Interest -- Liked Interest added
	
	string interest -- Name of the Interest to add
	
	string interest -- Name of the Interest added
*/
	
	IAddInterest() {}
	IAddInterest(const std::string& _i) : interest(_i) {}

	MAKE
		pack(interest);
	END_MAKE

	PARSE
		interest = unpack_string();
	END_PARSE

	std::string interest;
END

IFACEMESSAGE(IRemoveInterest, 0x0611)
/*
	Remove Interest -- Interest Removed
	
	string interest -- Name of the Interest to remove
	
	string interest -- Name of the Interest remove
*/
	
	IRemoveInterest() {}
	IRemoveInterest(const std::string& _i) : interest(_i) {}

	MAKE
		pack(interest);
	END_MAKE

	PARSE
		interest = unpack_string();
	END_PARSE

	std::string interest;
END

IFACEMESSAGE(IAddHatedInterest, 0x0612)
/*
	Add Hated Interest -- Hated Interest added
	
	string interest -- Name of the Interest to add
	
	string interest -- Name of the Interest added
*/
	
	IAddHatedInterest() {}
	IAddHatedInterest(const std::string& _i) : interest(_i) {}

	MAKE
		pack(interest);
	END_MAKE

	PARSE
		interest = unpack_string();
	END_PARSE

	std::string interest;
END


IFACEMESSAGE(IRemoveHatedInterest, 0x0613)
/*
	Remove Hated Interest -- Hated Interest removed
	
	string interest -- Name of the Interest to remove
	
	string interest -- Name of the Interest remove
*/
	
	IRemoveHatedInterest() {}
	IRemoveHatedInterest(const std::string& _i) : interest(_i) {}

	MAKE
		pack(interest);
	END_MAKE

	PARSE
		interest = unpack_string();
	END_PARSE

	std::string interest;
END



IFACEMESSAGE(IConnectServer, 0x0700)
/*
	Connect to Server -- Manually Connect to server
	
	*empty*
	
	*not sent*
*/
	
	IConnectServer() {}
	MAKE
	END_MAKE

	PARSE
	END_PARSE


END

IFACEMESSAGE(IDisconnectServer, 0x0701)
/*
	Disconnect from Server -- Manually Disconnects from server
	
	*empty*
	
	*not sent*
*/
	
	IDisconnectServer() {}
	MAKE
	END_MAKE

	PARSE
	END_PARSE


END

IFACEMESSAGE(IReloadShares, 0x0703)
/*
	Reload Shares -- Reload the Shares Database
	
	*empty*
	
	*not sent*
*/
	
	IReloadShares() {}
	MAKE
	END_MAKE

	PARSE
	END_PARSE
END

#endif // __IFACE_MESSAGES_HH__