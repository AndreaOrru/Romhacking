/***********************************************************************

S-DD1'inverse algorithm emulation code
--------------------------------------

Author:      Andreas Naive
Date:        August 2003
Last update: October 2004

This code is Public Domain. There is no copyright holded by the author.
Said this, the author wish to explicitly emphasize his inalienable moral rights
over this piece of intelectual work and the previous research that made it
possible, as recognized by most of the copyright laws around the world.

This code is provided 'as-is', with no warranty, expressed or implied.
No responsability is assumed by the author in connection with it.

The author is greatly indebted with The Dumper, without whose help and
patience providing him with real S-DD1 data the research would have never been
possible. He also wish to note that in the very beggining of his research,
Neviksti had done some steps in the right direction. By last, the author is
indirectly indebted to all the people that worked and contributed in the
S-DD1 issue in the past.

An algorithm's documentation is available as a separate document.
The implementation is obvious when the algorithm is
understood.

**************************************************************************/

#include "SDD1comp.h"

////////////////////////////////////////////////////////


void SDD1c_BE::putBit(uint8 bitplane) {

  std::vector<uint8> *currBPBuf = bitplaneBuffer[bitplane];
  uint8 *currBitInd = &bpBitInd[bitplane];

  if (!(*currBitInd)) currBPBuf->push_back(0);

  (*currBPBuf)[currBPBuf->size()-1] |=
    (((*inputBuffer) & (0x80 >> inBitInd)) << inBitInd) >> (*currBitInd);

  (*currBitInd)++;
  (*currBitInd) &= 0x07;

  inBitInd--;
  inBitInd &= 0x07;

}

////////////////////////////////////////////////////////


SDD1c_BE::SDD1c_BE(std::vector<uint8> *associatedBBuf0, std::vector<uint8> *associatedBBuf1,
		   std::vector<uint8> *associatedBBuf2, std::vector<uint8> *associatedBBuf3,
		   std::vector<uint8> *associatedBBuf4, std::vector<uint8> *associatedBBuf5,
		   std::vector<uint8> *associatedBBuf6, std::vector<uint8> *associatedBBuf7) {

  bitplaneBuffer[0]=associatedBBuf0;
  bitplaneBuffer[1]=associatedBBuf1;
  bitplaneBuffer[2]=associatedBBuf2;
  bitplaneBuffer[3]=associatedBBuf3;
  bitplaneBuffer[4]=associatedBBuf4;
  bitplaneBuffer[5]=associatedBBuf5;
  bitplaneBuffer[6]=associatedBBuf6;
  bitplaneBuffer[7]=associatedBBuf7;

}

////////////////////////////////////////////////////////


void SDD1c_BE::prepareComp(uint16 in_len, const uint8 *in_buf, uint8 header) {

  inputLength = in_len;
  inputBuffer = in_buf;
  bitplanesInfo = header & 0x0c;

}

////////////////////////////////////////////////////////


void SDD1c_BE::launch(void) {

  --inputLength;
  switch (bitplanesInfo) {
  case 0x00:
    currBitplane=1;
    break;
  case 0x04:
    currBitplane=7;
    break;
  case 0x08:
    currBitplane=3;
    break;
  case 0x0c:
    inBitInd=7;
    for (uint8 i=0; i<8; i++) bpBitInd[i]=0;
  }

  uint16 counter=0;
  do {

    switch(bitplanesInfo) {
    case 0x00:
      currBitplane ^= 0x01;
      bitplaneBuffer[currBitplane]->push_back(*(inputBuffer++));
      break;
    case 0x04:
      currBitplane ^= 0x01;
      if (!(counter & 0x000f)) currBitplane = ((currBitplane+2) & 0x07); 
      bitplaneBuffer[currBitplane]->push_back(*(inputBuffer++));
      break;
    case 0x08:
      currBitplane ^= 0x01;
      if (!(counter & 0x000f)) currBitplane ^= 0x02;
      bitplaneBuffer[currBitplane]->push_back(*(inputBuffer++));
      break;
    case 0x0c:
      for (uint8 i=0; i<8; i++) putBit(i);
      inputBuffer++;
    }

  } while (counter++<inputLength);


}

////////////////////////////////////////////////////////


SDD1c_CM::SDD1c_CM(std::vector<uint8> *associatedBBuf0, std::vector<uint8> *associatedBBuf1,
		   std::vector<uint8> *associatedBBuf2, std::vector<uint8> *associatedBBuf3,
		   std::vector<uint8> *associatedBBuf4, std::vector<uint8> *associatedBBuf5,
		   std::vector<uint8> *associatedBBuf6, std::vector<uint8> *associatedBBuf7) {

  bitplaneBuffer[0]=associatedBBuf0;
  bitplaneBuffer[1]=associatedBBuf1;
  bitplaneBuffer[2]=associatedBBuf2;
  bitplaneBuffer[3]=associatedBBuf3;
  bitplaneBuffer[4]=associatedBBuf4;
  bitplaneBuffer[5]=associatedBBuf5;
  bitplaneBuffer[6]=associatedBBuf6;
  bitplaneBuffer[7]=associatedBBuf7;

}

////////////////////////////////////////////////////////


void SDD1c_CM::prepareComp(uint8 header) {

  bitplanesInfo = header & 0x0c;
  contextBitsInfo = header & 0x03;
  for(int i=0; i<8; i++) {
    byte_ptr[i]=bitplaneBuffer[i]->begin();
    bpBitInd[i]=0;
    prevBitplaneBits[i]=0;
  }
  bit_number=0;
  switch (bitplanesInfo) {
  case 0x00:
    currBitplane=1;
    break;
  case 0x04:
    currBitplane=7;
    break;
  case 0x08:
    currBitplane=3;
  }
}

////////////////////////////////////////////////////////


uint8 SDD1c_CM::getBit(uint8 *context) {

  uint16 *context_bits;
  uint8 currContext;
  std::vector<uint8>::const_iterator *currBPBufIt;
  uint8 *currBitInd;
  uint8 bit;

  switch(bitplanesInfo) {
  case 0x00:
    currBitplane ^= 0x01;
    break;
  case 0x04:
    currBitplane ^= 0x01;
    if (!(bit_number & 0x7f)) currBitplane = ((currBitplane+2) & 0x07); 
    break;
  case 0x08:
    currBitplane ^= 0x01;
    if (!(bit_number & 0x7f)) currBitplane ^= 0x02;
    break;
  case 0x0c:
    currBitplane = bit_number & 0x07;
  }

  context_bits = &prevBitplaneBits[currBitplane];

  currContext=(currBitplane & 0x01)<<4;
  switch (contextBitsInfo) {
  case 0x00:
    currContext|=((*context_bits & 0x01c0)>>5)|(*context_bits & 0x0001);
    break;
  case 0x01:
    currContext|=((*context_bits & 0x0180)>>5)|(*context_bits & 0x0001);
    break;
  case 0x02:
    currContext|=((*context_bits & 0x00c0)>>5)|(*context_bits & 0x0001);
    break;
  case 0x03:
    currContext|=((*context_bits & 0x0180)>>5)|(*context_bits & 0x0003);
  }

  currBPBufIt = &byte_ptr[currBitplane];
  currBitInd = &bpBitInd[currBitplane];

  if (*currBPBufIt == bitplaneBuffer[currBitplane]->end()) bit = PEM->getMPS(currContext);
  else {
    bit = ((**currBPBufIt) & (0x80 >> *currBitInd))?1:0;
    if ((++*currBitInd) & 0x08) {
      *currBitInd = 0;
      (*currBPBufIt)++;
    }
  }

  *context_bits <<= 1;
  *context_bits |= bit;

  bit_number++;

  *context = currContext;
  return bit;

}

////////////////////////////////////////////////////////


uint8 SDD1c_PEM::getMPS(uint8 context) const{

  return contextInfo[context].MPS;

}

///////////////////////////////////////////////////////


SDD1c_PEM::SDD1c_PEM(SDD1c_CM *associatedCM, SDD1c_GCE *associatedGCE) :
  CM(associatedCM), GCE(associatedGCE)
{

  CM->PEM = this;

}

/////////////////////////////////////////////////////////


const SDD1c_PEM::state SDD1c_PEM::evolution_table[]={
    { 0,25,25},
    { 0, 2, 1},
    { 0, 3, 1},
    { 0, 4, 2},
    { 0, 5, 3},
    { 1, 6, 4},
    { 1, 7, 5},
    { 1, 8, 6},
    { 1, 9, 7},
    { 2,10, 8},
    { 2,11, 9},
    { 2,12,10},
    { 2,13,11},
    { 3,14,12},
    { 3,15,13},
    { 3,16,14},
    { 3,17,15},
    { 4,18,16},
    { 4,19,17},
    { 5,20,18},
    { 5,21,19},
    { 6,22,20},
    { 6,23,21},
    { 7,24,22},
    { 7,24,23},
    { 0,26, 1},
    { 1,27, 2},
    { 2,28, 4},
    { 3,29, 8},
    { 4,30,12},
    { 5,31,16},
    { 6,32,18},
    { 7,24,22}
  };

///////////////////////////////////////////////////////


void SDD1c_PEM::prepareComp(uint8 header, uint16 length) {

  for (uint8 i=0; i<32; i++) {
    contextInfo[i].status=0;
    contextInfo[i].MPS=0;
  }
  inputLength = length;
  if (((header & 0x0c) != 0x0c) && (length & 0x0001)) inputLength++;
  inputLength <<= 3;

}

//////////////////////////////////////////////////////


void SDD1c_PEM::launch(void) {

  bool8 endOfRun;
  uint8 bit;
  uint8 context;
  SDD1_ContextInfo *pContInfo;
  uint8 currStatus;
  const state *pState;

  for (uint32 i=0; i<inputLength; i++) {
    bit = CM->getBit(&context);
    pContInfo=&contextInfo[context];
    currStatus = pContInfo->status;
    pState=&SDD1c_PEM::evolution_table[currStatus];
    bit ^= pContInfo->MPS;
    GCE->putBit(pState->code_num, bit, &endOfRun);
    if (endOfRun)
      if (bit) {
	if (!(currStatus & 0xfe)) (pContInfo->MPS)^=0x01;
	(pContInfo->status)=pState->nextIfLPS;
      }
      else
	(pContInfo->status)=pState->nextIfMPS;
  }

  GCE->finishComp();

}

/////////////////////////////////////////////////////


void SDD1c_GCE::outputBit(uint8 code_num, bool8 bit) {

  uint8               &rBitInd      = bitInd[code_num];
  std::vector<uint8>* &rCodewBuffer = codewBuffer[code_num];
  
  uint8 oBit = (bit?0x80>>rBitInd:0x00);
  
  if (!rBitInd) (rCodewBuffer)->push_back(0);
  (*rCodewBuffer)[rCodewBuffer->size()-1] |= oBit;

  ++rBitInd;
  rBitInd &= 0x07;

}

/////////////////////////////////////////////////////


SDD1c_GCE::SDD1c_GCE
  (std::vector<uint8> *associatedCWSeq,
   std::vector<uint8> *associatedCWBuf0,
   std::vector<uint8> *associatedCWBuf1,
   std::vector<uint8> *associatedCWBuf2,
   std::vector<uint8> *associatedCWBuf3,
   std::vector<uint8> *associatedCWBuf4,
   std::vector<uint8> *associatedCWBuf5,
   std::vector<uint8> *associatedCWBuf6,
   std::vector<uint8> *associatedCWBuf7) :
  
  codewSequence(associatedCWSeq)
{
  codewBuffer[0] = associatedCWBuf0;
  codewBuffer[1] = associatedCWBuf1;
  codewBuffer[2] = associatedCWBuf2;
  codewBuffer[3] = associatedCWBuf3;
  codewBuffer[4] = associatedCWBuf4;
  codewBuffer[5] = associatedCWBuf5;
  codewBuffer[6] = associatedCWBuf6;
  codewBuffer[7] = associatedCWBuf7;
}

////////////////////////////////////////////////////


void SDD1c_GCE::prepareComp(void) {

  for (uint8 i=0; i<8; i++) {
    MPScount[i] = 0;
    bitInd[i] = 0;
  }

}

////////////////////////////////////////////////////


void SDD1c_GCE::putBit(uint8 code_num, uint8 bit, bool8 *endOfRun) {

  uint8 &rMPScount = MPScount[code_num];
     
  if (!rMPScount) codewSequence->push_back(code_num);
  
  if (bit) {
    *endOfRun = 1;
    outputBit(code_num, 1);
    for (uint8 i=0, aux=0x01; i<code_num; i++, aux<<=1)
      outputBit(code_num, !(rMPScount & aux));
    rMPScount = 0;
  }
  else {
    if (++(rMPScount) == (1<<code_num)) {      
      *endOfRun = 1;
      outputBit(code_num, 0);
      rMPScount = 0;
    }
    else
      *endOfRun = 0;
  }

}

////////////////////////////////////////////////////


void SDD1c_GCE::finishComp(void) {

  for (uint8 i=0; i<8; i++)
    if (MPScount[i])
      outputBit(i, 0);

}

////////////////////////////////////////////////////


bool8 SDD1c_I::moveBit(uint8 code_num) {

  uint8 *currBitInd = &bitInd[code_num];

  if (!oBitInd) *outputBuffer = 0;

  uint8 bit = (*byte_ptr[code_num] & (0x80 >> *currBitInd)) << *currBitInd;
  (*outputBuffer) |= bit >> oBitInd;

  if (++*currBitInd & 0x08) {
    *currBitInd = 0;
    byte_ptr[code_num]++;
  }
  if (++oBitInd & 0x08) {
    oBitInd = 0;
    outputBuffer++;
    ++*outputLength;
  }

  return bit;

}

///////////////////////////////////////////////////


SDD1c_I::SDD1c_I(std::vector<uint8> *associatedCWSeq,
		 std::vector<uint8> *associatedCWBuf0, std::vector<uint8> *associatedCWBuf1,
		 std::vector<uint8> *associatedCWBuf2, std::vector<uint8> *associatedCWBuf3,
		 std::vector<uint8> *associatedCWBuf4, std::vector<uint8> *associatedCWBuf5,
		 std::vector<uint8> *associatedCWBuf6, std::vector<uint8> *associatedCWBuf7) :
  codewSequence(associatedCWSeq)
{

  codewBuffer[0] = associatedCWBuf0;
  codewBuffer[1] = associatedCWBuf1;
  codewBuffer[2] = associatedCWBuf2;
  codewBuffer[3] = associatedCWBuf3;
  codewBuffer[4] = associatedCWBuf4;
  codewBuffer[5] = associatedCWBuf5;
  codewBuffer[6] = associatedCWBuf6;
  codewBuffer[7] = associatedCWBuf7;

}

////////////////////////////////////////////////////


void SDD1c_I::prepareComp(uint8 header, uint32 *out_len, uint8 *out_buf) {

  outputLength = out_len;
  *outputLength = 0;
  outputBuffer = out_buf;
  *outputBuffer=header<<4;
  oBitInd = 4;
  for (uint8 i=0; i<8; i++) {
    byte_ptr[i] = codewBuffer[i]->begin();
    bitInd[i] = 0;
  }
}

////////////////////////////////////////////////////


void SDD1c_I::launch(void) {

    for (std::vector<uint8>::const_iterator p=codewSequence->begin(); p != codewSequence->end(); p++)
      if (moveBit(*p))
	for (uint8 i=0; i<*p; i++) moveBit(*p);

    if (oBitInd) ++*outputLength;

}

////////////////////////////////////////////////////


SDD1comp::SDD1comp(void) :
  BE(&bitplaneBuffer[0], &bitplaneBuffer[1], &bitplaneBuffer[2], &bitplaneBuffer[3],
     &bitplaneBuffer[4], &bitplaneBuffer[5], &bitplaneBuffer[6], &bitplaneBuffer[7]),
  CM(&bitplaneBuffer[0], &bitplaneBuffer[1], &bitplaneBuffer[2], &bitplaneBuffer[3],
     &bitplaneBuffer[4], &bitplaneBuffer[5], &bitplaneBuffer[6], &bitplaneBuffer[7]),
  GCE(&codewordsSequence,
      &codewordBuffer[0], &codewordBuffer[1], &codewordBuffer[2], &codewordBuffer[3],
      &codewordBuffer[4], &codewordBuffer[5], &codewordBuffer[6], &codewordBuffer[7]),
  PEM(&CM, &GCE),
  I(&codewordsSequence,
    &codewordBuffer[0], &codewordBuffer[1], &codewordBuffer[2], &codewordBuffer[3],
    &codewordBuffer[4], &codewordBuffer[5], &codewordBuffer[6], &codewordBuffer[7])
{

}

///////////////////////////////////////////////////


void SDD1comp::compress(uint16 in_len, const uint8 *in_buf, uint32 *out_len, uint8 *out_buf) {

  uint32 min_length;
  uint8 *buffer;

  compress(0, in_len, in_buf, out_len, out_buf);

  min_length = *out_len;
  buffer = new uint8[*out_len];
  for (uint32 i=0; i<*out_len; i++) buffer[i] = out_buf[i];

  for (uint8 j=1; j<16; j++) {
    compress(j, in_len, in_buf, out_len, out_buf);
    if (*out_len < min_length) {
      min_length = *out_len;
      for (uint32 i=0; i<*out_len; i++) buffer[i] = out_buf[i];
    }
  }

  if (min_length < *out_len) {
    *out_len = min_length;
    for (uint32 i=0; i<min_length; i++) out_buf[i] = buffer[i];
  }

  delete[] buffer;

}

///////////////////////////////////////////////////


void SDD1comp::compress(uint8 header, uint16 in_len, const uint8 *in_buf, uint32 *out_len, uint8 *out_buf) {

  //Step 1
  for (uint8 i=0; i<8; i++) bitplaneBuffer[i].clear();
  BE.prepareComp(in_len, in_buf, header);
  BE.launch();

  //Step 2
  codewordsSequence.clear();
  for (uint8 i=0; i<8; i++)
    codewordBuffer[i].clear();
  CM.prepareComp(header);
  PEM.prepareComp(header, in_len);
  GCE.prepareComp();
  PEM.launch();

  //Step 3
  I.prepareComp(header, out_len, out_buf);
  I.launch();

}

//////////////////////////////////////////////////
