## DESCRIPTION

Client & Server side application that communicate between each other via HTTP and REST API.

## OPTIONS

Start client:

    $ ./ftrest COMMAND REMOTE-PATH [LOCAL-PATH]

Start server:

    $ ./ftrestd [-r ROOT-FOLDER] [-p PORT]


## REST COMMANDS

Upload folder to server:

    PUT /USER-ACCOUNT/REMOTE-PATH?type=[folder] HTTP/1.1

Download data from server:

    GET /USER-ACCOUNT/REMOTE-PATH?type=[file|folder] HTTP/1.1

Delete data from server:

    DELETE /USER-ACCOUNT/REMOTE-PATH?type=[file|folder] HTTP/1.1


## RETURN VALUES

REST:

Successful operation:

    200 OK

Folder not found:

    404 Not Found

Bad Request operation:

    400 Bad Request

On client:

Commands lst, rmd:

    Not a directory.

Commands lst, rmd:

    Directory not found.

Command rmdir:

    Directory not empty.

Commands mkd, put:

    Already exists.

Commands del, get:

    Not a file.

Commands del, get:

    File not found.

Other errors:

    Unknown error.

## SYNTAX

Example commands:

Make directory bar on server:

    $ ftrest mkd http://localhost:12345/tonda/foo/bar


Delete file doc.pdf on server:

    $ ftrest del http://localhost:12345/tonda/foo/bar/doc.pdf

## COPYRIGHT

Marián Mrva, FIT VUT Brno



