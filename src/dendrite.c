#include "dendrite.h"
#include "funcs.h"
#include "stdio.h"










inline SDR256 toSDR(Dendrite* d){
  SDR256 ret;
  for(int i = 0; i < 4; i++) ret.bits[i] = 0;

  for(int i = 0; i < 8; i++){
    int shift = 4 * (i % 2);
    if(((d->weights[i/2] >> shift) % 16) >= SYNAPSETHRESHOLD){
      int bitindex = d->pos[i] / 64;
      int bitoffset= d->pos[i] % 64;
      ret.bits[bitindex] |= (1 << bitoffset);
    }
  }
  return ret;
}










int match(Dendrite* d, SDR256 sdr, int threshold){
  SDR256 dend = toSDR(d);

  SDR256 inter= sdr256_intersection(dend, sdr);

  int count = popcount(inter.bits[0]) + popcount(inter.bits[1]) + popcount(inter.bits[2]) + popcount(inter.bits[3]);

  return count >= threshold;
}










void learn(Dendrite* d, SDR256 sdr, int threshold){

  if(match(d, sdr, threshold)){

    SDR256 dend = toSDR(d); // Probably not efficient to recalculate this, but KISS for now.

    int reassign = 0; // Bit vector

    for(int i = 0; i < 8; i++){
      int shift = 4 * (i % 2);
      uint8_t weight = (d->weights[i/2] >> shift) & 0x0F;

      int bitindex = d->pos[i] / 64;
      int bitoffset= d->pos[i] % 64;

      if(sdr.bits[bitindex] & dend.bits[bitindex] & (1 << bitoffset)){
        //Increase weight
        if(weight != 0x0F){
          weight++;
          d->weights[i/2] &= (0x00F0 >> shift);
          d->weights[i/2] |= (weight << shift);
        }
      }else{
        //Decrease weight, alert to reassigns
        if(weight != 0x00){
          weight--;
          d->weights[i/2] &= (0x00F0 >> shift);
          d->weights[i/2] |= (weight << shift);
        }else{
          reassign |= (1 << i);
        }
      }
    }

    // Are there synapses that need to be reassigned?
    if(reassign){

      SDR256 unusedBits;

      // Get bits that are in the input sdr, but not on the dendrite
      for(int i = 0; i < 4; i++)
        unusedBits.bits[i] = ~dend.bits[i] & sdr.bits[i];

      /*
        Not quite sure how I'll do this yet, but I need to figure it out
      */
    }
  }

  // If the SDR doesn't match the dendrite, no learning occurs.
  return;
}










void showDendrite(Dendrite* d){
  SDR256 sdr = toSDR(d);
  sdr256_show(sdr);
  printf("%i\n", sdr256_count(sdr));

  for(int i = 0; i < 8; i++){
    int weight = (d->weights[i/2] >> (4 * (i % 2))) % 16;
    printf("  %i @ %i\n", d->pos[i], weight);
  }
}










void randDendrite(Dendrite* d){
  for(int i = 0; i < 8; i++)
    d->pos[i] = fastRand(); // Synapses are random

  for(int i = 0; i < 4; i++)
    d->weights[i] = 0x44;   // but synapses are weak and quick to change
}
