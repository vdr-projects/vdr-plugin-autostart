/*
 * vdrsimulator.h: Simulates two vdr calls, so that the media detector
 *                 test programm can be build without vdr.
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef VDRSIMULATOR_H_
#define VDRSIMULATOR_H_

#include <string.h>

enum eKeys { // "Up" and "Down" must be the first two keys!
             kUp,
             kDown,
             kMenu,
             kOk,
             kBack,
             kLeft,
             kRight,
             kRed,
             kGreen,
             kYellow,
             kBlue,
             k0, k1, k2, k3, k4, k5, k6, k7, k8, k9,
             kNone,
           };

struct tKey {
  eKeys type;
  const char *name;
};
static tKey keyTable[] = { // "Up" and "Down" must be the first two keys!
                    { kUp,            "Up"         },
                    { kDown,          "Down"       },
                    { kMenu,          "Menu"       },
                    { kOk,            "Ok"         },
                    { kBack,          "Back"       },
                    { kLeft,          "Left"       },
                    { kRight,         "Right"      },
                    { kRed,           "Red"        },
                    { kGreen,         "Green"      },
                    { kYellow,        "Yellow"     },
                    { kBlue,          "Blue"       },
                    { k0,                    "0"                },
                    { k1,                    "1"                },
                    { k2,                    "2"                },
                    { k3,                    "3"                },
                    { k4,                    "4"                },
                    { k5,                    "5"                },
                    { k6,                    "6"                },
                    { k7,                    "7"                },
                    { k8,                    "8"                },
                    { k9,                    "9"                },
                    { kNone,                 NULL               },
                  };

class cKey
{
public:
    // Simulate the availablity of a reduced set of keys.
    static eKeys FromString(const char *Name)
    {
        if (Name) {
            for (tKey *k = keyTable; k->name; k++) {
                const char *n = k->name;
                const char *p = strchr(n, '$');
                if (p)
                    n = p + 1;
                if (strcasecmp(n, Name) == 0)
                    return k->type;
            }
        }
        return kNone;
    }
};

typedef char cPlugin;

class cPluginManager
{
public:
    // Simulate the availablity of the following plugins.
    static cPlugin *GetPlugin(char const *plugin) {
        if ((strcmp (plugin,"music") == 0) ||
            (strcmp (plugin,"image") == 0) ||
            (strcmp (plugin,"externalplayer") == 0) ||
            (strcmp (plugin,"cdplayer") == 0)) {
            return (cPlugin *)plugin;
        }
        return NULL;
    }
};

#endif /* VDRSIMULATOR_H_ */
