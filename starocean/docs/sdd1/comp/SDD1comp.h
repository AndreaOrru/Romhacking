/**********************************************************************

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

*************************************************************************/

#include <vector>

class SDD1c_BE { //Bitplanes Extractor

 public:
  SDD1c_BE(std::vector<uint8> *associatedBBuf0, std::vector<uint8> *associatedBBuf1,
	   std::vector<uint8> *associatedBBuf2, std::vector<uint8> *associatedBBuf3,
	   std::vector<uint8> *associatedBBuf4, std::vector<uint8> *associatedBBuf5,
	   std::vector<uint8> *associatedBBuf6, std::vector<uint8> *associatedBBuf7);
  void prepareComp(uint16 in_len, const uint8 *in_buf, uint8 header);
  void launch(void);

 private:
  uint8 bitplanesInfo;
  uint16 inputLength;
  const uint8 *inputBuffer;
  uint8 inBitInd;
  uint8 currBitplane;
  std::vector<uint8> *bitplaneBuffer[8];
  uint8 bpBitInd[8];
  void putBit(uint8 bitplane);

};

///////////////////////////////////////////////////////


class SDD1c_PEM;

class SDD1c_CM { //Context Model

  friend class SDD1c_PEM;

 public:
  SDD1c_CM(std::vector<uint8> *associatedBBuf0, std::vector<uint8> *associatedBBuf1,
	   std::vector<uint8> *associatedBBuf2, std::vector<uint8> *associatedBBuf3,
	   std::vector<uint8> *associatedBBuf4, std::vector<uint8> *associatedBBuf5,
	   std::vector<uint8> *associatedBBuf6, std::vector<uint8> *associatedBBuf7);
  void prepareComp(uint8 header);
  uint8 getBit(uint8 *context);

 private:
  uint8 bitplanesInfo;
  uint8 contextBitsInfo;
  std::vector<uint8> *bitplaneBuffer[8];
  std::vector<uint8>::const_iterator byte_ptr[8];
  uint8 bpBitInd[8];
  uint8 bit_number;
  uint8 currBitplane;
  uint16 prevBitplaneBits[8];
  SDD1c_PEM *PEM;

};

///////////////////////////////////////////////////////


class SDD1c_GCE;

class SDD1c_PEM { //Probability Estimation Module

 public:
  SDD1c_PEM(SDD1c_CM *associatedCM, SDD1c_GCE *associatedGCE);

  void prepareComp(uint8 header, uint16 length);
  uint8 getMPS(uint8 context) const;
  void launch(void);

 private:
  struct state {
    uint8 code_num;
    uint8 nextIfMPS;
    uint8 nextIfLPS;
  };
  static const state evolution_table[];
  struct SDD1_ContextInfo {
    uint8 status;
    uint8 MPS;
  } contextInfo[32];
  SDD1c_CM *const CM;
  SDD1c_GCE *const GCE;
  uint32 inputLength;

};

//////////////////////////////////////////////////////


class SDD1c_GCE { //Golomb-Code Encoder

 public:
  SDD1c_GCE(std::vector<uint8> *associatedCWSeq,
            std::vector<uint8> *associatedCWBuf0,
            std::vector<uint8> *associatedCWBuf1,
            std::vector<uint8> *associatedCWBuf2,
            std::vector<uint8> *associatedCWBuf3,
            std::vector<uint8> *associatedCWBuf4,
            std::vector<uint8> *associatedCWBuf5,
            std::vector<uint8> *associatedCWBuf6,
            std::vector<uint8> *associatedCWBuf7);
  void prepareComp(void);
  void putBit(uint8 code_num, uint8 bit, bool8 *endOfRun);
  void finishComp(void);

 private:
  std::vector<uint8> *const codewSequence;
  std::vector<uint8> * codewBuffer[8];
  uint8 bitInd[8];
  uint8 MPScount[8];
  void outputBit(uint8 code_num, bool8 bit);
  
};

/////////////////////////////////////////////////////


class SDD1c_I { //Interleaver

 public:
  SDD1c_I(std::vector<uint8> *associatedCWSeq,
	  std::vector<uint8> *associatedCWBuf0, std::vector<uint8> *associatedCWBuf1,
	  std::vector<uint8> *associatedCWBuf2, std::vector<uint8> *associatedCWBuf3,
	  std::vector<uint8> *associatedCWBuf4, std::vector<uint8> *associatedCWBuf5,
	  std::vector<uint8> *associatedCWBuf6, std::vector<uint8> *associatedCWBuf7);
  void prepareComp(uint8 header, uint32 *out_len, uint8 *out_buf);
  void launch(void);

 private:
  std::vector<uint8> *const codewSequence;
  std::vector<uint8> *codewBuffer[8];
  std::vector<uint8>::const_iterator byte_ptr[8];
  uint8 bitInd[8];
  uint32 *outputLength;
  uint8 *outputBuffer;
  uint8 oBitInd;
  bool8 moveBit(uint8 code_num);

};

///////////////////////////////////////////////////////////


class SDD1comp {

 public:
  SDD1comp(void);
  void compress(uint16 in_len, const uint8 *in_buf, uint32 *out_len, uint8 *out_buf);
  void compress(uint8 header, uint16 in_len, const uint8 *in_buf, uint32 *out_len, uint8 *out_buf);

 private:
  std::vector<uint8> bitplaneBuffer[8];
  std::vector<uint8> codewordsSequence;
  std::vector<uint8> codewordBuffer[8];
  SDD1c_BE BE;
  SDD1c_CM CM;
  SDD1c_GCE GCE;
  SDD1c_PEM PEM;
  SDD1c_I I;

};
