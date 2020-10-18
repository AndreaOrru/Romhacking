MTE OPTIMIZATION:
  python2 mteOpt.py -s <ita-script.txt> -d dict.txt -c clean.txt -m 3 -M 11 -l 128 -o 0

GENERATE BINARY:
  ./generate_bins.py

APPLY MTE TO SCRIPT:
  ./apply_dict.py <ita-script.txt> <dict.txt> <encoded-script.txt>

ASSEMBLE:
  asar extended_mte.asm smrpg.sfc
