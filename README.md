# cdidc
A simple C program to calculate MusicBrainz and CDDB IDs of a CD.
Almost everything is actually done by `libdiscid`, this is a nice wrapper for it.


## Usage
Print all IDs:

```
$ cdidc
MusicBrainz Disc ID: 60h_eoFMGeWtBr45lfvY50pFW7I-
CDDB ID: 940a450b
```

Print just the MusicBrainz Disc ID:

```
$ cdidc -bm
60h_eoFMGeWtBr45lfvY50pFW7I-
```

For more information, check the man page or run `cdidc -h`.


## Dependencies
- [libdiscid](https://musicbrainz.org/doc/libdiscid)


## Compilation and installing
To compile, run `make`.
To install, run `make install`.


## License
Copyright (C) 2021 Riku Viitanen (riku.viitanen at protonmail.com).

Licensed under the [GNU GPL version 3](https://www.gnu.org/licenses/gpl-3.0.html) or (at your option) any later version.
See the file 'COPYING' for the complete license text.

