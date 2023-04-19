package org.vaultdb.plan.operator;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.logging.Logger;

import org.apache.calcite.plan.RelOptUtil;
import org.apache.calcite.rel.RelWriter;
import org.apache.calcite.rel.externalize.RelJsonWriter;
import org.apache.calcite.rel.logical.LogicalJoin;
import org.apache.calcite.rex.RexNode;
import org.apache.calcite.rex.RexUtil;
import org.apache.calcite.sql.SqlExplainFormat;
import org.apache.calcite.sql.SqlExplainLevel;
import org.apache.commons.lang3.builder.HashCodeBuilder;
import org.vaultdb.config.SystemConfiguration;
import org.vaultdb.config.ExecutionMode;
import org.vaultdb.plan.SecureRelNode;
import org.vaultdb.type.SecureRelDataTypeField;
import org.vaultdb.type.SecureRelRecordType;
import org.vaultdb.type.SecureRelDataTypeField.SecurityPolicy;
import org.vaultdb.util.Utilities;

// for execution mode planning over a RelNode
public abstract class Operator{

  protected SecureRelNode baseRelNode;
  protected List<Operator> children;

  protected Operator parent;

  Logger logger;

  String operatorId;
ExecutionMode executionMode = null;
  String queryName;

  SystemConfiguration config;


  public Operator(String name, SecureRelNode src, Operator... childOps) throws Exception {
    baseRelNode = src;
    src.setPhysicalNode(this);
    children = new ArrayList<Operator>();
    config = SystemConfiguration.getInstance();

    operatorId = config.getOperatorId();

    for (Operator op : childOps) {
      children.add(op);
      op.setParent(this);
    }

    logger = config.getLogger();
    queryName = name.replaceAll("-", "_");


  }

  public void inferExecutionMode() throws Exception {
    for (Operator op : children) {
      op.inferExecutionMode();
    }

    ExecutionMode maxChild = maxChildMode();
    SecurityPolicy maxAccess = maxAccessLevel(); // most sensitive attribute it computes on

    executionMode = new ExecutionMode(); // defaults to distributed oblivious, i.e. MPC
    executionMode.replicated = getSchema().isReplicated(); // inferred this during schema resolution

    String msg =
        "For "
            + baseRelNode.getRelNode().getRelTypeName()
            + " have max child "
            + maxChild
            + " and max access "
            + maxAccess;
    logger.info(msg);

    // if maxChild is DistributedClear or LocalClear and maxAccess is public,
    // then set it equal to maxChild

    if (!maxChild.distributed) {
      if (!maxChild.oblivious) { // local-clear
        if (maxAccess == SecurityPolicy.Public) {
          executionMode.distributed = false;
          executionMode.oblivious = false; // sliced init'd t false
        } else { // oblivious child
          try {
            if (locallyRunnable()) {
              executionMode.distributed = false;
            }
          } catch (Exception e) {
            e.printStackTrace();
            System.exit(-1);
          }
        }
      } else // max child is local-oblivious
      if (locallyRunnable()) {
        executionMode.distributed = false;
      }
    }

  }

  // print subtree for this and descendants
  protected String printSubtree() {

    return appendOperator(this, new String(), "");
  }

  String appendOperator(Operator op, String src, String linePrefix) {
    src += linePrefix + op + "\n";
    linePrefix += "    ";
    for (Operator child : op.getChildren()) {
      src = appendOperator(child, src, linePrefix);
    }
    return src;
  }

  // checks to see if we can use replication, partitioned-alike attributes, or unary ops
  // to run this locally
  protected boolean locallyRunnable() throws Exception {

    boolean local = true;

    for (Operator child :
        children) { // either child is replicated or if binary op, have one input that is replicated
      if (child.getSchema().isReplicated()) return true;
    }

    if (this instanceof Filter || this instanceof Project || this instanceof SeqScan) { // unary ops
      return true;
    }

    // if not partitioned-by well-known attrs
    if (!(config.getProperty("has-partitioning") == null
        || config.getProperty("has-partitioning").equals("true"))) {
      return false;
    }

    // a binary op where the instances are partitioned-alike
    if (children.size() == 2) {

      if (!(this instanceof Join)) {
        throw new Exception("Only binary operator supported is join!");
      }

      LogicalJoin join = (LogicalJoin) baseRelNode.getRelNode();
      List<RexNode> joinCondition = join.getChildExps();
      com.google.common.collect.ImmutableList<RexNode> predicates = RexUtil.flattenAnd(joinCondition);

      Operator lhsChild = children.get(0);
      Operator rhsChild = children.get(1);
      SecureRelRecordType lhsSchema = lhsChild.getSchema();
      SecureRelRecordType rhsSchema = rhsChild.getSchema();

      if (lhsSchema.isReplicated() || rhsSchema.isReplicated()) // automatically partitioned-alike
      return true;

      List<SecureRelDataTypeField> lhsFields = lhsSchema.getAttributes();
      List<SecureRelDataTypeField> rhsFields = rhsSchema.getAttributes();

      for (RexNode predicate : predicates) {
        SecureRelDataTypeField lhsPartitionBy = null;
        SecureRelDataTypeField rhsPartitionBy = null;

        List<SecureRelDataTypeField> srcAttrs =
            AttributeResolver.getAttributes(predicate, this.getSchema());

        for (SecureRelDataTypeField aField : srcAttrs) {

          if (lhsFields.contains(aField)) {
            if (lhsPartitionBy == null) {
              lhsPartitionBy =
                  aField; // get first field from lhs that we compute on, assumes equality
              // predicates
            } else {
              throw new Exception("Composite partitioning keys not yet implemented!");
            }
          }

          // Always get the Unaliased name since we are checking against the base relation.
          // Aliasing happens here: SqlValidatorUtil.java:557 (calcite:1.18.0)
          SecureRelDataTypeField rhsField =
              new SecureRelDataTypeField(
                  aField.getUnaliasedName(),
                  aField.getIndex() - lhsFields.size(),
                  aField.getType());
          if (rhsFields.contains(rhsField)) {
            if (rhsPartitionBy == null) {
              rhsPartitionBy = aField; // get first field from rhs that we compute on
            } else {
              throw new Exception("Composite partitioning keys not yet implemented!");
            }
          }
        } // end schema matcher

      } // end for each predicate
      return local;
    } // end if binary op


    // end unary case
    return local;
  }


  public void addChild(Operator op) {
    children.add(op);
    op.setParent(this);
  }

  public void addChildren(List<Operator> ops) {
    children.addAll(ops);
    for (Operator op : ops) op.setParent(this);
  }

  public void setParent(Operator op) {
    parent = op;
  }

  public SecureRelNode getSecureRelNode() {
    return baseRelNode;
  }

  public List<Operator> getChildren() {
    return children;
  }
  
  public int getNumChildren() {
	  if (children == null) return 0;
	  
	  return children.size();
  }

  // what fields does this operator reveal information on?
  // some, like SeqScan and Project reveal nothing in their output based on the contents of the
  // tuples they process
  // thus they "computeOn" nothing
  public List<SecureRelDataTypeField> computesOn() {
    return new ArrayList<SecureRelDataTypeField>();
  }

  protected SecurityPolicy maxAccessLevel() {
    List<SecureRelDataTypeField> accessed = computesOn();
    SecurityPolicy policy = SecurityPolicy.Public;

    for (SecureRelDataTypeField field : accessed) {
      if (field.getSecurityPolicy().compareTo(policy) > 0) policy = field.getSecurityPolicy();
    }

    return policy;
  }

  protected ExecutionMode maxChildMode() {
    ExecutionMode maxMode = new ExecutionMode(children.get(0).executionMode);

    // join or other binary op
    if (children.size() == 2) {
      ExecutionMode rhs = children.get(1).executionMode;
      if (rhs.distributed) maxMode.distributed = true;

      if (rhs.oblivious) maxMode.oblivious = true;
    }

    return maxMode;
  }

  // schema of output
  public SecureRelRecordType getSchema() {
    return baseRelNode.getSchema();
  }

  // schema of output
  public SecureRelRecordType getSchema(boolean asSecureLeaf) {
    return baseRelNode.getSchema();
  }


  // for all but SeqScan and join
  public SecureRelRecordType getInSchema() {
    return children.get(0).getSchema();
  }

  // for all except sort
  public int getLimit() {
    return -1;
  }



  public String toString() {

     return   baseRelNode.getRelNode().getRelTypeName() + "-" + executionMode + ", schema:" + getSchema();
  }


  public ExecutionMode getExecutionMode() {
    return executionMode;
  }

  public Operator getChild(int i) {
    return children.get(i);
  }

  public String getOpName() {
    String fullName = this.getClass().toString();
    int startIdx = fullName.lastIndexOf('.') + 1;
    return fullName.substring(startIdx);
  }

  public String getOperatorId() {
    return operatorId;
  }
  public String getQueryId() {
    return queryName;
  }



  public Operator getParent() {
    return parent;
  }

  @Override
  public boolean equals(Object o) {
    if (!(o instanceof Operator)) return false;
    return this.toString().equals(o.toString());
  }


  @Override
  public int hashCode() {
    return new HashCodeBuilder(17, 31).append(this.toString()).toHashCode();
  }

  // if a MPC operator implementation leverages the order of tuples for comparisons, codify that in
  // this function
  // otherwise empty set = order agnostic
  // order is implicitly ascending
  public List<SecureRelDataTypeField> secureComputeOrder() {
    return new ArrayList<SecureRelDataTypeField>();
  }


}

