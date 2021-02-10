NAME=cdidc
VERSION=0.1
AUTHOR="Riku Viitanen"

CFLAGS=-Wall
OPTLEVEL=2

CFILES=cdidc.c
POFILES=$(wildcard l10n/*.po)
MOFILES=$(patsubst l10n/%.po,build/l10n/%.mo,$(POFILES))
MANPAGE_TEMPLATES=$(wildcard man/man*/*) $(wildcard man/*/man*/*)
MANPAGES=$(patsubst %,build/%.gz,$(MANPAGE_TEMPLATES))

LIBS=-ldiscid


all: build/$(NAME) l10n man


build/$(NAME): $(CFILES)
	mkdir -p build
	cc -s $(CFLAGS) $(LIBS) -O$(OPTLEVEL) -DNAME=\"$(NAME)\" -DVERSION=\"$(VERSION)\" -o $@ $^


debug: build/$(NAME)-debug l10n man
build/$(NAME)-debug: $(CFILES)
	mkdir -p build
	cc $(CFLAGS) $(LIBS) -g -DNAME=\"$(NAME)\" -DVERSION=\"$(VERSION)\" -o build/$(NAME)-debug $^


man: $(MANPAGES)

build/man/%.gz: man/%
	mkdir -p build/man
	echo $^
	echo $@ | awk -F/ -v OFS=/ "{NF=NF-1; print}" | xargs mkdir -p
	sed "s/VERSION_NUMBER/$(VERSION)/g" $^ | gzip -9 - > $@


l10n: $(MOFILES)

build/l10n/%.mo: l10n/%.po
	mkdir -p build/l10n
	msgfmt $^ -o $@


l10n-update-template: $(CFILES)
	xgettext --from-code UTF-8 --package-name $(NAME) --package-version=$(VERSION) --copyright-holder=$(AUTHOR) --output l10n/template.pot -k_ $(CFILES)


run: $(NAME)
	./$(NAME)


install: build/$(NAME) l10n
	cp build/$(NAME) /usr/bin/
	cp -r build/man/* /usr/share/man/
	@ls -1 build/l10n/*.mo | sed -E "p;s/build\/l10n\/(.*).mo/\/usr\/share\/locale\/\1\/LC_MESSAGES\/$(NAME).mo/" | xargs -L2 cp

uninstall:
	rm -f /usr/bin/$(NAME)
	rm /usr/share/locale/*/LC_MESSAGES/$(NAME).mo


clean:
	rm -rf build/*
