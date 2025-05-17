INPUT = README.md
OUTPUT = README.pdf

PANDOC_FLAGS = --pdf-engine=pdflatex -f markdown+escaped_line_breaks -V lang=cs \
			   -V margin-left=20mm -V margin-top=20mm -V margin-right=20mm \
			   -V margin-bottom=20mm -V colorlinks=true -V urlcolor=DarkRed \
			   --toc --highlight-style=tango -M default-language=plaintext \
			   --to latex

all: $(OUTPUT)

$(OUTPUT): $(INPUT)
	pandoc $(PANDOC_FLAGS) -o $(OUTPUT) $(INPUT)

clean:
	rm -f $(OUTPUT)
