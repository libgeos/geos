import os
import sys
import distutils.util
import math

# Put local build directory on head of python path
platformdir = '-'.join((distutils.util.get_platform(),
                        '.'.join(map(str, sys.version_info[0:2]))))
sys.path.insert(0, os.path.join('build', 'lib.' + platformdir))

# import geos from the local build directory
import geos

pm = geos.PrecisionModel(2.0,0,0)
global_factory = geos.GeometryFactory(pm,-1)

def wkt_print_geoms(geoms):
    wkt = geos.WKTWriter()
    size = len(geoms)
    for i in range(size):
        tmp = wkt.write(geoms[i])
        print "[%d]" % i, tmp

def create_point(x,y):
    c = geos.Coordinate(x,y)
    p = global_factory.createPoint(c)

    return p

def create_ushaped_linestring(xoffset, yoffset, side):
    cl = geos.DefaultCoordinateSequence()

    cl.add(geos.Coordinate(xoffset, yoffset))
    cl.add(geos.Coordinate(xoffset, yoffset+side))
    cl.add(geos.Coordinate(xoffset+side, yoffset+side))
    cl.add(geos.Coordinate(xoffset+side, yoffset))

    ls = global_factory.createLineString(cl)
    return ls

def create_square_linearring(xoffset,yoffset,side):
    cl = geos.DefaultCoordinateSequence()

    cl.add(geos.Coordinate(xoffset,yoffset))
    cl.add(geos.Coordinate(xoffset,yoffset+side))
    cl.add(geos.Coordinate(xoffset+side,yoffset+side))
    cl.add(geos.Coordinate(xoffset+side,yoffset))
    cl.add(geos.Coordinate(xoffset,yoffset))

    lr = global_factory.createLinearRing(cl)
    return lr

def create_square_polygon(xoffset,yoffset,side):
    outer = create_square_linearring(xoffset,yoffset,side)
    inner = create_square_linearring(xoffset+(side/3.),yoffset+(side/3.),(side/3.))

    holes = geos.vector_GeometryP()
    holes.push_back(inner)

    poly = global_factory.createPolygon(outer,holes)
    return poly

def create_simple_collection(geoms):
    collect = geos.vector_GeometryP()
    for i in geoms:
        collect.push_back(i)
    return global_factory.createGeometryCollection(collect)

def create_circle(centerX,centerY,radius):
    shapefactory = geos.GeometricShapeFactory(global_factory)
    shapefactory.setCentre(geos.Coordinate(centerX, centerY))
    shapefactory.setSize(radius)
    return shapefactory.createCircle()

def create_ellipse(centerX,centerY,width,height):
    shapefactory = geos.GeometricShapeFactory(global_factory)
    shapefactory.setCentre(geos.Coordinate(centerX, centerY))
    shapefactory.setHeight(height)
    shapefactory.setWidth(width)
    return shapefactory.createCircle()

def create_rectangle(llX,llY,width,height):
    shapefactory = geos.GeometricShapeFactory(global_factory)
    shapefactory.setBase(geos.Coordinate(llX, llY))
    shapefactory.setHeight(height)
    shapefactory.setWidth(width)
    shapefactory.setNumPoints(4)
    return shapefactory.createRectangle()

def create_arc(llX,llY,width,height,startang,endang):
    shapefactory = geos.GeometricShapeFactory(global_factory)
    shapefactory.setBase(geos.Coordinate(llX, llY))
    shapefactory.setHeight(height)
    shapefactory.setWidth(width)
    #shapefactory.setNumPoints(100) #the default (100 pts)
    return shapefactory.createArc(startang, endang)

def do_all():
    geoms = []

    geoms.append(create_point(150, 350))
    geoms.append(create_ushaped_linestring(60,60,100))
    geoms.append(create_square_linearring(0,0,100))
    geoms.append(create_square_polygon(0,200,300))
    geoms.append(create_square_polygon(0,250,300))
    geoms.append(create_simple_collection(geoms))

    # These ones use a GeometricShapeFactory
    geoms.append(create_circle(0, 0, 10))
    geoms.append(create_ellipse(0, 0, 8, 12))
    geoms.append(create_rectangle(-5, -5, 10, 10)) # a square
    geoms.append(create_rectangle(-5, -5, 10, 20)) # a rectangle

    # The upper-right quarter of a vertical ellipse
    geoms.append(create_arc(0, 0, 10, 20, 0, math.pi/2))

    print "--------HERE ARE THE BASE GEOMS ----------"
    wkt_print_geoms(geoms)


####################
# UNARY OPERATIONS                 #
####################

    ################
    #CENTROID                        #
    ################
    # Find centroid of each base geometry
    newgeoms = []
    for i in range(len(geoms)):
        newgeoms.append(geoms[i].getCentroid())

    print "\n","------- AND HERE ARE THEIR CENTROIDS -----"
    wkt_print_geoms(newgeoms)

    ################
    # BUFFER                           #
    ################
    newgeoms = []
    for i in range(len(geoms)):
        try:
            newgeoms.append(geoms[i].buffer(10))
        except geos.GEOSException():
            exc = geos.GEOSException()
            print "GEOS Exception: geometry ",geoms[i],"->buffer(10): ",exc.toString()

    print "\n","--------HERE COMES THE BUFFERED GEOMS ----------"
    wkt_print_geoms(newgeoms)

    ################
    # CONVEX HULL                 #
    ################
    newgeoms = []
    for i in range(len(geoms)):
        newgeoms.append(geoms[i].convexHull())

    print "\n","--------HERE COMES THE HULLS----------"
    wkt_print_geoms(newgeoms)

####################
# RELATIONAL OPERATORS        #
####################

    print "-------------------------------------------------------------------------------"
    print "RELATIONAL OPERATORS"
    print "-------------------------------------------------------------------------------"

    size = len(geoms)
    ################
    # DISJOINT                          #
    ################

    print
    print "\t".join(["   DISJOINT   "]+["[%d]" % i for i in range(size)])

    for i in range(size):
        print "      [%d]\t" % i,
        for j in range(size):
            try:
                if geoms[i].disjoint(geoms[j]):
                    print " 1\t",
                else:
                    print " 0\t",
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                print " X\t",
        print

    ################
    # TOUCHES                         #
    ################

    print
    print "\t".join(["    TOUCHES   "]+["[%d]" % i for i in range(size)])

    for i in range(size):
        print "      [%d]\t" % i,
        for j in range(size):
            try:
                if geoms[i].touches(geoms[j]):
                    print " 1\t",
                else:
                    print " 0\t",
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                print " X\t",
        print

    ################
    # INTERSECTS                   #
    ################

    print
    print "\t".join([" INTERSECTS   "]+["[%d]" % i for i in range(size)])

    for i in range(size):
        print "      [%d]\t" % i,
        for j in range(size):
            try:
                if geoms[i].intersects(geoms[j]):
                    print " 1\t",
                else:
                    print " 0\t",
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                print " X\t",
        print

    ################
    # CROSSES                        #
    ################

    print
    print "\t".join(["    CROSSES   "]+["[%d]" % i for i in range(size)])

    for i in range(size):
        print "      [%d]\t" % i,
        for j in range(size):
            try:
                if geoms[i].crosses(geoms[j]):
                    print " 1\t",
                else:
                    print " 0\t",
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                print " X\t",
        print

    ################
    # WITHIN                        #
    ################

    print
    print "\t".join(["     WITHIN   "]+["[%d]" % i for i in range(size)])

    for i in range(size):
        print "      [%d]\t" % i,
        for j in range(size):
            try:
                if geoms[i].within(geoms[j]):
                    print " 1\t",
                else:
                    print " 0\t",
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                print " X\t",
        print

    ################
    # CONTAINS                        #
    ################

    print
    print "\t".join(["   CONTAINS   "]+["[%d]" % i for i in range(size)])

    for i in range(size):
        print "      [%d]\t" % i,
        for j in range(size):
            try:
                if geoms[i].contains(geoms[j]):
                    print " 1\t",
                else:
                    print " 0\t",
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                print " X\t",
        print

    ################
    # OVERLAPS                        #
    ################

    print
    print "\t".join(["   OVERLAPS   "]+["[%d]" % i for i in range(size)])

    for i in range(size):
        print "      [%d]\t" % i,
        for j in range(size):
            try:
                if geoms[i].overlaps(geoms[j]):
                    print " 1\t",
                else:
                    print " 0\t",
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                print " X\t",
        print

    ################
    # RELATE                            #
    ################

    print
    print "\t".join(["     RELATE   "]+["[%d]" % i for i in range(size)])

    for i in range(size):
        print "      [%d]\t" % i,
        for j in range(size):
            im = geos.IntersectionMatrix('')
            try:
                if geoms[i].relate(geoms[j],"212101212"):
                    print " 1\t",
                else:
                    print " 0\t",
                im=geoms[i].relate(geoms[j])
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                print " X\t",
        print

    ################
    # EQUALS                        #
    ################

    print
    print "\t".join(["     EQUALS   "]+["[%d]" % i for i in range(size)])

    for i in range(size):
        print "      [%d]\t" % i,
        for j in range(size):
            try:
                if geoms[i].equals(geoms[j]):
                    print " 1\t",
                else:
                    print " 0\t",
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                print " X\t",
        print

    ################
    # EQUALS_EXACT             #
    ################

    print
    print "\t".join(["EQUALS_EXACT  "]+["[%d]" % i for i in range(size)])

    for i in range(size):
        print "      [%d]\t" % i,
        for j in range(size):
            try:
                if geoms[i].equalsExact(geoms[j],0.5):
                    print " 1\t",
                else:
                    print " 0\t",
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                print " X\t",
        print

    ################
    # IS_WITHIN_DISTANCE     #
    ################

    print
    print "\t".join(["IS_WITHIN_DIST"]+["[%d]" % i for i in range(size)])

    for i in range(size):
        print "      [%d]\t" % i,
        for j in range(size):
            try:
                if geoms[i].isWithinDistance(geoms[j],2):
                    print " 1\t",
                else:
                    print " 0\t",
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                print " X\t",
        print

####################
# COMBINATIONS
####################

    print
    print "-------------------------------------------------------------------------------"
    print "COMBINATIONS"
    print "-------------------------------------------------------------------------------"

    ################
    # UNION
    ################

    newgeoms = []
    for i in range(size-1):
        for j in range(i+1,size):
            try:
                newgeoms.append(geoms[i].Union(geoms[j]))
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                pass

    print "\n", "----- AND HERE ARE SOME UNION COMBINATIONS ------"
    wkt_print_geoms(newgeoms)

    ################
    # INTERSECTION
    ################

    newgeoms = []
    for i in range(size-1):
        for j in range(i+1,size):
            try:
                newgeoms.append(geoms[i].intersection(geoms[j]))
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                pass

    print "\n", "----- HERE ARE SOME INTERSECTIONS COMBINATIONS ------"
    wkt_print_geoms(newgeoms)

    ################
    # DIFFERENCE
    ################

    newgeoms = []
    for i in range(size-1):
        for j in range(i+1,size):
            try:
                newgeoms.append(geoms[i].difference(geoms[j]))
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                pass

    print "\n", "----- HERE ARE SOME DIFFERENCE COMBINATIONS ------"
    wkt_print_geoms(newgeoms)

    ################
    # SYMMETRIC DIFFERENCE
    ################

    newgeoms = []
    for i in range(size-1):
        for j in range(i+1,size):
            try:
                newgeoms.append(geoms[i].symDifference(geoms[j]))
            except geos.GEOSException():
                exc = geos.GEOSException()
                print exc.toString()
            except:
                pass

    print "\n", "----- HERE ARE SYMMETRIC DIFFERENCES ------"
    wkt_print_geoms(newgeoms)

    ################
    # LINEMERGE
    ################
    temp = geos.vector_GeometryP()
    for g in geoms:
        temp.push_back(g)
    lm = geos.LineMerger()
    lm.add(temp)
    mls = lm.getMergedLineStrings()
    newgeoms = []
    for i in range(mls.size()):
        newgeoms.append(mls[i])
    del mls

    print "\n", "----- HERE IS THE LINEMERGE OUTPUT ------"
    wkt_print_geoms(newgeoms)
    
    ################
    # POLYGONIZE
    ################
    temp = geos.vector_GeometryP()
    for g in geoms:
        temp.push_back(g)
    plgnzr = geos.Polygonizer()
    plgnzr.add(temp)
    polys = plgnzr.getPolygons()
    newgeoms = []
    for i in range(polys.size()):
        newgeoms.append(polys[i])
    del polys

    print "\n", "----- HERE IS POLYGONIZE OUTPUT ------"
    wkt_print_geoms(newgeoms)

print "GEOS", geos.geosversion(), "ported from JTS", geos.jtsport()
do_all()
