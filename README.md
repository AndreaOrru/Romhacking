# Romhacking
Tools, hacks, technical info about good old games.

## Installation
### Asar (65c816 assembler)
```bash
git clone https://github.com/RPGHacker/asar
cd asar
cmake src && make
cp asar/asar-standalone /usr/local/bin/asar
```

### Python dependencies
```bash
# Install dependencies inside a virtualenv.
# i.e.: mkvirtualenv romhacking
pip install -r requirements.txt
```

## Usage
```bash
# Extract.
./bin/evertool extract evermore.sfc dump.txt

# Reinsert (creates patched_evermore.sfc).
./bin/evertool reinsert evermore.sfc dump.txt
```

## Tests
```bash
# Install dev dependencies.
pip install -r requirements-dev.txt

# Make sure the original ROMs are in the test directories.
cp evermore.sfc evermore/tests/roms/
cp so.sfc starocean/tests/roms/

# Run tests.
pytest                 # Globally.
cd evermore && pytest  # Per package.
```
