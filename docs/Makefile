PDF_ENGINE = pdflatex
PANDOC_FLAGS =--pdf-engine=$(PDF_ENGINE) -V margin-left=20mm -V margin-top=20mm \
			   -V margin-right=20mm -V margin-bottom=20mm -V colorlinks=true \
			   -V urlcolor=DarkRed --toc --highlight-style=tango \
			   -M default-language=plaintext -V lang=en --to latex

all: monolog.pdf

monolog.pdf: monolog.md
	pandoc $(PANDOC_FLAGS) -o monolog.pdf monolog.md

clean:
	rm -f monolog.pdf
