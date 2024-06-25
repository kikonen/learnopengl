// Decode utility functions

uint decode16x1(uint v)
{
  return v & 0xffff;
}

uint decode16x2(uint v)
{
  return (v >> 16) & 0xffff;
}

uint decodeMaterialIndex(uint v)
{
  return decode16x2(v);
}

uint decodeShapeIndex(uint v)
{
  return decode16x1(v);
}
