Please follow these guidelines or have a look at the source code:

###General:###

- Indent mode: **spaces**, Indent size: **4**.
- max **80** chars for row.
- separate each funtion by **//------------------------------------------------------------------------------**

###Class Members:###

- use prefix **m** for variables followed by a camel case name eg.: **QProgressDialog *mProgressDialog;**.
- use prefix **k** for costants followed by a camel case name eg.: **kVersionUrl**.
- use prefix **t** inside the function for variables followed by a camel case name eg.: **QUrl tUrl(kVersionUrl);**.

###Class Funtions:###

- use prefix **on_** followed by a camel case name eg.: **void on_CancelDownload();** for slots.
- use a fisrt word lower case followed by a camel case name eg.: **void checkForUpdates();**.

to be continued...
