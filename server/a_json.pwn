// Copyright 2012 Las Venturas Playground. All rights reserved.
// Use of this source code is governed by the GPLv2 license, a copy of which can
// be found in the LICENSE file.

/**
 * Import the native functions required for fully supporing the JSON library. This relies on the
 * JSONReader plugin to be installed and made available.
 */
native JSON_parse(filename[]);
native JSON_next(Node: node);
native JSON_find(Node: node, name[]);
native JSON_firstChild(Node: node);
native JSON_getName(Node: node, buffer[], bufferSize);
native JSON_getType(Node: node);
native JSON_readString(Node: node, buffer[], bufferSize);
native JSON_readInteger(Node: node, &value);
native JSON_readFloat(Node: node, &Float: value);
native JSON_readBoolean(Node: node, &bool: value);
native JSON_close();
