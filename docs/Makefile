PDF_ENGINE = pdflatex
PANDOC_FLAGS =--pdf-engine=$(PDF_ENGINE) -f markdown+escaped_line_breaks -V lang=cs \
			   -V margin-left=20mm -V margin-top=20mm -V margin-right=20mm \
			   -V margin-bottom=20mm -V colorlinks=true -V urlcolor=DarkRed \
			   --toc --highlight-style=tango -M default-language=plaintext \
			   --to latex

all: README-cs.pdf

README-cs.pdf: README-cs.md
	pandoc $(PANDOC_FLAGS) -o README-cs.pdf README-cs.md

clean:
	rm -f README-cs.pdf
