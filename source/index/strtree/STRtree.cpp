/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.2  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


//This class is not used anywhere, thus not ported.


///**
// *  An R-tree created using the Sort-Tile-Recursive (STR) algorithm, described
// *  in: P. Rigaux, Michel Scholl and Agnes Voisard. Spatial Databases With
// *  Application To GIS. Morgan Kaufmann, San Francisco, 2002. <P>
// *
// *  The STR packed R-tree is simple to implement and maximizes space
// *  utilization; that is, as many leaves as possible are filled to capacity.
// *  Overlap between nodes is far less than in a basic R-tree. However, once the
// *  tree has been built (explicitly or on the first call to #query), items may
// *  not be added or removed. <P>
// *
// *  This implementation is based on Rectangles rather than Nodes, because the
// *  STR algorithm operates on both nodes and items, both of which are treated
// *  here as Rectangles (using the Composite design pattern). [Jon Aquino]
// */
//public class STRtree extends AbstractSTRtree implements SpatialIndex {
//
//  private Comparator xComparator =
//    new Comparator() {
//      public int compare(Object o1, Object o2) {
//        return compareDoubles(
//            centreX((Envelope)((Boundable)o1).getBounds()),
//            centreX((Envelope)((Boundable)o2).getBounds()));
//      }
//    };
//  private Comparator yComparator =
//    new Comparator() {
//      public int compare(Object o1, Object o2) {
//        return compareDoubles(
//            centreY((Envelope)((Boundable)o1).getBounds()),
//            centreY((Envelope)((Boundable)o2).getBounds()));
//      }
//    };
//
//  private double centreX(Envelope e) {
//    return avg(e.getMinX(), e.getMaxX());
//  }
//
//  private double avg(double a, double b) { return (a + b) / 2d; }
//
//  private double centreY(Envelope e) {
//    return avg(e.getMinY(), e.getMaxY());
//  }
//
//  private IntersectsOp intersectsOp = new IntersectsOp() {
//    public boolean intersects(Object aBounds, Object bBounds) {
//      return ((Envelope)aBounds).intersects((Envelope)bBounds);
//    }
//  };
//
//  protected Comparator getComparator() {
//    return yComparator;
//  }
//
//  /**
//   * Creates the parent level for the given child level. First, orders the items
//   * by the x-values of the midpoints, and groups them into vertical slices.
//   * For each slice, orders the items by the y-values of the midpoints, and
//   * group them into runs of size M (the node capacity). For each run, creates
//   * a new (parent) node.
//   */
//  protected List createParentBoundables(List childBoundables, int newLevel) {
//    Assert.isTrue(!childBoundables.isEmpty());
//    int minLeafCount = (int) Math.ceil((childBoundables.size() / (double) getNodeCapacity()));
//    ArrayList sortedChildBoundables = new ArrayList(childBoundables);
//    Collections.sort(sortedChildBoundables, xComparator);
//    List[] verticalSlices = verticalSlices(sortedChildBoundables,
//        (int) Math.ceil(Math.sqrt(minLeafCount)));
//    return createParentBoundablesFromVerticalSlices(verticalSlices, newLevel);
//  }
//
//  private List createParentBoundablesFromVerticalSlices(List[] verticalSlices, int newLevel) {
//    Assert.isTrue(verticalSlices.length > 0);
//    List parentBoundables = new ArrayList();
//    for (int i = 0; i < verticalSlices.length; i++) {
//      parentBoundables.addAll(
//            createParentBoundablesFromVerticalSlice(verticalSlices[i], newLevel));
//    }
//    return parentBoundables;
//  }
//
//  protected List createParentBoundablesFromVerticalSlice(List childBoundables, int newLevel) {
//    return super.createParentBoundables(childBoundables, newLevel);
//  }
//
//  /**
//   * @param childBoundables Must be sorted by the x-value of the envelope midpoints
//   * @return
//   */
//  protected List[] verticalSlices(List childBoundables, int sliceCount) {
//    int sliceCapacity = (int) Math.ceil(childBoundables.size() / (double) sliceCount);
//    List[] slices = new List[sliceCount];
//    Iterator i = childBoundables.iterator();
//    for (int j = 0; j < sliceCount; j++) {
//      slices[j] = new ArrayList();
//      int boundablesAddedToSlice = 0;
//      while (i.hasNext() && boundablesAddedToSlice < sliceCapacity) {
//        Boundable childBoundable = (Boundable) i.next();
//        slices[j].add(childBoundable);
//        boundablesAddedToSlice++;
//      }
//    }
//    return slices;
//  }
//
//  public STRtree() { this(10); }
//
//  public STRtree(int nodeCapacity) {
//    super(nodeCapacity);
//  }
//
//  protected AbstractNode createNode(int level) {
//    return new AbstractNode(level) {
//      protected Object computeBounds() {
//        Envelope bounds = null;
//        for (Iterator i = getChildBoundables().iterator(); i.hasNext(); ) {
//          Boundable childBoundable = (Boundable) i.next();
//          if (bounds == null) {
//            bounds = new Envelope((Envelope)childBoundable.getBounds());
//          }
//          else {
//            bounds.expandToInclude((Envelope)childBoundable.getBounds());
//          }
//        }
//        return bounds;
//      }
//    };
//  }
//
//  protected IntersectsOp getIntersectsOp() {
//    return intersectsOp;
//  }
//
//  public void insert(Envelope itemEnv, Object item) {
//    if (itemEnv.isNull()) { return; }
//    super.insert(itemEnv, item);
//  }
//
//  public List query(Envelope searchEnv) {
//    return super.query(searchEnv);
//  }
//
//}
