import click

from starocean.rom import ROM


@click.group()
def cli():
    """Star Ocean (SNES DeJap) text extractor/reinserter."""
    ...


@cli.command()
@click.argument('rom', type=click.Path(exists=True))
@click.argument('dump', type=click.Path())
def extract(rom: str, dump: str):
    """Extract all text from the original ROM."""
    dump_text = ROM(rom).extract()
    with open(dump, 'w', encoding='utf-8') as dump_file:
        dump_file.write(dump_text)


@cli.command()
@click.argument('rom', type=click.Path(exists=True))
@click.argument('dump', type=click.Path(exists=True))
def reinsert(rom: str, dump: str):
    """Generate a ROM with reinserted text."""
    with open(dump, encoding='utf-8') as dump_file:
        dump = dump_file.read()
    ROM(rom).reinsert(dump)


if __name__ == '__main__':
    cli(prog_name='sotool')
