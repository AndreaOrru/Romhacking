# -*- coding: utf-8 -*-

import sys, os, re, io
from shutil import copyfile
from collections import defaultdict, Counter, OrderedDict

import argparse
parser = argparse.ArgumentParser()
parser.add_argument('-m', '--min', action='store', dest='min', type=int, required=True, help='')
parser.add_argument('-M', '--max',  action='store', dest='max', type=int, required=True, help='')
parser.add_argument('-l', '--limit',  action='store', dest='limit', type=int, required=True, help='')
parser.add_argument('-s', '--source', action='store', dest='source_file', required=True, help='Original filename')
parser.add_argument('-c', '--clean', action='store', dest='clean_file', required=True, help='Clean filename')
parser.add_argument('-d', '--dest',  action='store', dest='dest_file', required=True, help='Destination filename')
parser.add_argument('-o', '--offset',  action='store', dest='offset', type=int, required=True, help='Starting offset')
args = parser.parse_args()

MIN = args.min
MAX = args.max
LIMIT = args.limit
filename = args.source_file
filename1 = args.clean_file
filename2 = args.dest_file
offset = args.offset

SMRPG_REGEX_LIST = [
	(re.compile(r'^.{7}'), ''),
	(re.compile(r' {5,}'), ''),
	(re.compile(r'[.]{3,}'), '\n'),
	(re.compile(r'\[.+?\]'), '\n')
]

def clean_file(filename, filename1, regexList=[], allowDuplicates=True):
	""" applica a ogni linea del file delle espressioni regolari, rimuove opzionalmente le linee duplicate """
	with io.open(filename, mode='r', encoding="utf-8") as f, io.open(filename1, mode='w', encoding="utf-8") as f1:
		linesSeen = set()
		for line in f.readlines():
			for regex in regexList:
				line = regex[0].sub(regex[1], line)
			if allowDuplicates:
				f1.write(line)
			elif line not in linesSeen:
				f1.write(line)
				linesSeen.add(line)

def extract_chunks(text, chunkSize, startPoint=0):
	return [text[i:(i+chunkSize)] for i in range(startPoint, len(text), chunkSize)]

def get_substrings_by_length(text, length):
	chunks = []
	if length > 0:
		for i in range (0, length):
			chunks += extract_chunks(text, length, i)
	return list(filter(lambda x: len(x) == length, chunks))

def get_occurrences_by_length(filename, length):
	dictionary = defaultdict(int)
	with io.open(filename, mode='r', encoding="utf-8") as f:
		for line in f.readlines():
			if line:
				line = line.replace('\n', '').replace('\r', '')
				substrings = get_substrings_by_length(line, length)
				for string in substrings: dictionary[string] += 1
	return dictionary

def get_occurrences(filename, min_length, max_length):
	dictionary = Counter()
	for length in range(min_length, max_length + 1):
		occurrences = Counter(get_occurrences_by_length(filename, length))
		dictionary.update(occurrences)
	return dictionary

def calculate_weight(dictionary):
	""" crea un dizionario pesato sulla lunghezza delle parole """
	return {k: v * (len(k) - 2) for k, v in dictionary.items()}

def sort_dict_by_value(dictionary, reverse=True):
	""" reversa il dizionario e lo ordina per valore """
	return sorted(dictionary.iteritems(), key=lambda(k,v):(v,k), reverse=reverse)

# def extractWordsFromFile(filename):
# 	""" estrae tutte le parole (sequenze separate da uno spazio) da un file """
# 	words = []
# 	with io.open(filename, mode='r', encoding="utf-8") as f:
# 		for line in f.readlines():
# 			wordsToAdd = line.replace('\n', '').replace('\r', '').replace("'", " ").split(" ")
# 			wordsToAdd = filter(None, wordsToAdd)
# 			# for word in wordsToAdd:
# 			# 	if word + ' ' in line:
# 			# 		wordsToAdd.append(word + ' ')
# 			# 		wordsToAdd.remove(word)
# 			words += wordsToAdd
# 	return words

# def extractLinesFromFile(filename):
# 	""" estrae tutte le linee da un file """
# 	lines = []
# 	with io.open(filename, mode='r', encoding="utf-8") as f:
# 		lines = f.readlines()
# 	return lines

# def wordRegexFilter(words, regexList):
# 	""" ripulisce le parole da caretteri e byte inutili """
# 	if words and regexList:
# 		for regex in regexList:
# 			words = list(map(lambda x: regex[0].sub(regex[1], x), words))
# 		words = list(filter(lambda x: x and x != '', words))
# 	return words if words else []

# def syllableCounter(list, min=2, max=3):
# 	"""  """
# 	dictionary = {}
# 	if list:
# 		for elem in list:
# 			if len(elem) >= min:
# 				for i in range(min, max+1):
# 					for k in range(0, len(elem)):
# 						syllable = elem[k:k+i]
# 						if len(syllable) >= i:
# 							if syllable not in dictionary:
# 								dictionary[syllable] = 0
# 							dictionary[syllable] = dictionary[syllable] + 1
# 	return dictionary

# def wordCounter(words, min=2, max=3):
# 	"""  """
# 	dictionary = {}
# 	if words:
# 		for word in words:
# 			if len(word) in range(min, max+1):
# 				if word not in dictionary:
# 					dictionary[word] = 0
# 				dictionary[word] = dictionary[word] + 1
# 	return dictionary

clean_file(filename, filename1, regexList=SMRPG_REGEX_LIST, allowDuplicates=False)
#print get_substrings_by_length(x, 3)
#print get_occurrences_by_length(filename1, 3)
dictionary = OrderedDict()
curr_filename = filename1
for i in range(0, LIMIT):
	next_filename = filename1 + '.' + str(i)
	xx = get_occurrences(curr_filename, MIN, MAX)
	xxx = calculate_weight(xx)
	xxxx = sort_dict_by_value(xxx)
	k, v = xxxx[0]
	dictionary[k] = v
	print k
	clean_file(curr_filename, next_filename, regexList=[(re.compile(re.escape(k)), '\n')], allowDuplicates=True)
	curr_filename = next_filename
	# print xxxx[:LIMIT]
	#print dictionary[u' che ']
	#sys.exit()
print dictionary

# words = extractWordsFromFile(filename1)
# ##words = extractLinesFromFile(filename1)
# #print("---------")
# filteredWords = wordRegexFilter(words, None)
# #print("---------")
# #print(filteredWords)
# dictionary = wordCounter(filteredWords, min=MIN, max=MAX)
# ##dictionary = syllableCounter(filteredWords, min=MIN, max=MAX)
# #print("---------")
# #print(dictionary)
# weight_dictionary = calculate_weight(dictionary)
# #print("---------")
# #print(weight_dictionary)
# weight_dictionary_by_value = sort_dict_by_value(weightDictionary)
# #print weight_dictionary_by_value
# #print weight_dictionary_by_value[:LIMIT]
# with io.open(filename2, mode='w', encoding="utf-8") as out:
# 	for i, e in enumerate(weight_dictionary_by_value[:LIMIT]):
# 		n = hex(i + offset).rstrip('L')
# 		b = (n + '').replace('0x', '')
# 		b = b.zfill(4)
# 		line = "%s=%s" % (b, e[0])
# 		out.write(line + '\n')
