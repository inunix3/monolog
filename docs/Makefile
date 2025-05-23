PDF_ENGINE = pdflatex
PANDOC_FLAGS =--pdf-engine=$(PDF_ENGINE) -V margin-left=20mm -V margin-top=20mm \
			   -V margin-right=20mm -V margin-bottom=20mm -V colorlinks=true \
			   -V urlcolor=DarkRed --toc --highlight-style=tango \
			   -M default-language=plaintext --to latex

PANDOC_FLAGS_CS = $(PANDOC_FLAGS) -V lang=cs
PANDOC_FLAGS_EN = $(PANDOC_FLAGS) -V lang=en

all: monolog-cs.pdf monolog.pdf

monolog-cs.pdf: monolog-cs.md
	pandoc $(PANDOC_FLAGS_CS) -o monolog-cs.pdf monolog-cs.md

monolog.pdf: monolog.md
	pandoc $(PANDOC_FLAGS_EN) -o monolog.pdf monolog.md

clean:
	rm -f monolog-cs.pdf monolog.pdf
