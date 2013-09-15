This repository contains some TrueType tools by Rogier van Dalen. This is an unofficial copy of the sources from http://home.kabelfoon.nl/~slam/fonts/source.html, updated for the modern Linux distro system.

-----------------

List of predefined functions in TTIComp
---------------------------------------

```glsl
void setCVT(uint label, fixed value); // pixels
void setCVT(uint label, int value);   // not pixels

fixed getCVT(uint label);

void setVectorsX();
void setProjectionX();
void setFreedomX();
void setVectorsY();
void setProjectionY();
void setFreedomY();

void setProjection(uint x1, uint y1, uint x2, uint y2);
void setProjectionPerp(uint x1, uint y1, uint x2, uint y2);
void setDualProjection(uint x1, uint y1, uint x2, uint y2);
void setDualProjectionPerp(uint x1, uint y1, uint x2, uint y2);
void setFreedom(uint x1, uint y1, uint x2, uint y2);
void setFreedomPerp(uint x1, uint y1, uint x2, uint y2);

void setFreedomProjection();

void setRoundHalf();
void setRoundGrid();
void setRoundDouble();
void setRoundDown();
void setRoundUp();
void setRoundOff();

void setRound45(uint n); // Super ROUND 45 degrees
void setRound(uint n); // Super ROUND

void setMinDist(fixed distance);
void setInstructionExecution(uint selector, uint value);
void setScanConversion(uint n);
void setScanType(uint n);
void setCVTCutIn(fixed n);
void setSingleWidth(fixed n);
void setSingleWidthCutIn(fixed n);
void setAutoFlipOn();
void setAutoFlipOff();
void setDeltaBase(uint n);
void setDeltaShift(uint n);

uint getPPEM();
uint getPointSize();

fixed getCoordinate(uint zone, uint point);
fixed getCoordinateOrig(uint zone, uint point);
void setCoordinate(uint zone, uint point, fixed distance);

fixed getDistance(uint zone1, uint point1, uint zone2, uint point2);
fixed getDistanceOrig(uint zone1, uint point1, uint zone2, uint point2);

void flipPoint(uint point);
void flipRangeOn(uint lowPoint, uint highPoint);
void FlipRangeOff(uint lowPoint, uint highPoint); // Yes capital F.

void shiftPoint(uint zoneRef, uint pointRef, uint zone, uint point);
void shiftContour(uint zoneRef, uint pointRef, uint contour);
void shiftZone(uint zoneRef, uint pointRef, uint zone);
void shiftPoint(uint zone, uint point, fixed magnitude);

void touch(uint zone, uint point);
void roundPoint(uint zone, uint point);
void movePoint(uint zone, uint point, uint cvtEntry);
void movePointRound(uint zone, uint point, uint cvtEntry);
void moveDistance(uint zoneRef, uint pointRef, uint zone, uint point, fixed distance);

void moveDistanceWhite(uint zoneRef, uint pointRef, uint zone, uint point);
void moveDistanceGrey(uint zoneRef, uint pointRef, uint zone, uint point);
void moveDistanceBlack(uint zoneRef, uint pointRef, uint zone, uint point);
void moveDistanceRoundWhite(uint zoneRef, uint pointRef, uint zone, uint point);
void moveDistanceRoundGrey(uint zoneRef, uint pointRef, uint zone, uint point);
void moveDistanceRoundBlack(uint zoneRef, uint pointRef, uint zone, uint point);
void moveDistanceMinDistWhite(uint zoneRef, uint pointRef, uint zone, uint point);
void moveDistanceMinDistGrey(uint zoneRef, uint pointRef, uint zone, uint point);
void moveDistanceMinDistBlack(uint zoneRef, uint pointRef, uint zone, uint point);
void moveDistanceMinDistRoundWhite(uint zoneRef, uint pointRef, uint zone, uint point);
void moveDistanceMinDistRoundGrey(uint zoneRef, uint pointRef, uint zone, uint point);
void moveDistanceMinDistRoundBlack(uint zoneRef, uint pointRef, uint zone, uint point);

void moveDistanceWhite(uint zoneRef, uint pointRef, uint zone, uint point, uint cvtEntry);
void moveDistanceGrey(uint zoneRef, uint pointRef, uint zone, uint point, uint cvtEntry);
void moveDistanceBlack(uint zoneRef, uint pointRef, uint zone, uint point, uint cvtEntry);
void moveDistanceRoundWhite(uint zoneRef, uint pointRef, uint zone, uint point, uint cvtEntry);
void moveDistanceRoundGrey(uint zoneRef, uint pointRef, uint zone, uint point, uint cvtEntry);
void moveDistanceRoundBlack(uint zoneRef, uint pointRef, uint zone, uint point, uint cvtEntry);
void moveDistanceMinDistWhite(uint zoneRef, uint pointRef, uint zone, uint point, uint cvtEntry);
void moveDistanceMinDistGrey(uint zoneRef, uint pointRef, uint zone, uint point, uint cvtEntry);
void moveDistanceMinDistBlack(uint zoneRef, uint pointRef, uint zone, uint point, uint cvtEntry);
void moveDistanceMinDistRoundWhite(uint zoneRef, uint pointRef, uint zone, uint point, uint cvtEntry);
void moveDistanceMinDistRoundGrey(uint zoneRef, uint pointRef, uint zone, uint point, uint cvtEntry);
void moveDistanceMinDistRoundBlack(uint zoneRef, uint pointRef, uint zone, uint point, uint cvtEntry);

void align(uint zoneRef, uint pointRef, uint zone, uint point);
void intersection(uint zoneA, uint a0, uint a1, uint zoneB, uint b0, uint b1, uint zone, uint point);
void alignPoints(uint zone1, uint point1, uint zone2, uint point2);
void interpolate(uint zone1, uint point1, uint zone2, uint point2, uint zone, uint point);

void untouch(uint zone, uint point);
void interpolateX();
void interpolateY();

void deltaP1(uint point, uint arg);
void deltaP2(uint point, uint arg);
void deltaP3(uint point, uint arg);
void deltaC1(uint point, uint arg);
void deltaC2(uint point, uint arg);
void deltaC3(uint point, uint arg);

bool odd(fixed n);
bool even(fixed n);
fixed abs(fixed n);
int abs(int n);
fixed floor(fixed n);
int floor(int n);
fixed ceiling(fixed n);
int ceiling(int n);
uint min(uint x, uint y);
uint max(uint x, uint y);

fixed roundWhite(fixed n);
fixed roundGrey(fixed n);
fixed roundBlack(fixed n);
fixed compensateWhite(fixed n);
fixed compensateGrey(fixed n);
fixed compensateBlack(fixed n);

uint getInformation(uint selector);
```

