#ifndef PKUNK_H
#define PKUNK_H

#ifndef FRASMODINFO
#define FRASMODINFO

typedef struct{
  const unsigned length;
  const unsigned fineTune;
  const unsigned linearVolume;
  const unsigned repeatOffset;
  const char     *sampleData;
} sampleInfo;

typedef struct{
  const sampleInfo     *sample;
  const unsigned short period;
  const unsigned short effect;
} patternChannelInfo;

typedef struct{
  const unsigned   nrOfPatterns;
  const unsigned   nrOfChannels;
  const patternChannelInfo *patternTable[];
} modInfo;

#endif

extern const modInfo pkunkModInfo;

#endif
