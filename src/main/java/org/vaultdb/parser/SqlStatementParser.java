package org.vaultdb.parser;

import java.util.logging.Logger;

import org.apache.calcite.adapter.java.JavaTypeFactory;
import org.apache.calcite.config.CalciteConnectionConfig;
import org.apache.calcite.config.Lex;
import org.apache.calcite.jdbc.CalciteConnection;
import org.apache.calcite.jdbc.CalciteSchema;
import org.apache.calcite.jdbc.JavaTypeFactoryImpl;
import org.apache.calcite.prepare.CalciteCatalogReader;
import org.apache.calcite.prepare.PlannerImpl;
import org.apache.calcite.prepare.Prepare;
import org.apache.calcite.prepare.Prepare.CatalogReader;
import org.apache.calcite.rel.RelNode;
import org.apache.calcite.rel.RelRoot;
import org.apache.calcite.rel.logical.LogicalFilter;
import org.apache.calcite.rel.rules.ReduceExpressionsRule;
import org.apache.calcite.rel.type.RelDataTypeFactory;
import org.apache.calcite.rel.type.RelDataTypeSystem;
import org.apache.calcite.rex.RexBuilder;
import org.apache.calcite.rex.RexNode;
import org.apache.calcite.rex.RexUtil;
import org.apache.calcite.schema.SchemaPlus;
import org.apache.calcite.sql.SqlExplainFormat;
import org.apache.calcite.sql.SqlExplainLevel;
import org.apache.calcite.sql.SqlNode;
import org.apache.calcite.sql.SqlOperatorTable;
import org.apache.calcite.sql.ddl.SqlCreateTable;
import org.apache.calcite.sql.dialect.PostgresqlSqlDialect;
import org.apache.calcite.sql.parser.SqlParseException;
import org.apache.calcite.sql.parser.SqlParser;
import org.apache.calcite.sql.parser.ddl.SqlDdlParserImpl;
import org.apache.calcite.sql.validate.SqlConformance;
import org.apache.calcite.sql.validate.SqlConformanceEnum;
import org.apache.calcite.sql.validate.SqlValidator;
import org.apache.calcite.sql.validate.SqlValidatorCatalogReader;
import org.apache.calcite.sql.validate.SqlValidatorImpl;
import org.apache.calcite.sql2rel.SqlToRelConverter;
import org.apache.calcite.sql2rel.SqlToRelConverter.Config;
import org.apache.calcite.sql2rel.StandardConvertletTable;
import org.apache.calcite.tools.FrameworkConfig;
import org.apache.calcite.tools.Planner;
import org.apache.calcite.tools.RelConversionException;
import org.apache.calcite.tools.ValidationException;
import org.vaultdb.config.SystemConfiguration;
import org.apache.calcite.plan.Context;
import org.apache.calcite.plan.RelOptCluster;
import org.apache.calcite.plan.RelOptUtil;
import org.apache.calcite.plan.hep.HepPlanner;
import org.apache.calcite.plan.hep.HepProgramBuilder;
import org.apache.calcite.rel.rules.*;


// parse and validate a sql statement against a schema
public class SqlStatementParser {
	
	SchemaPlus sharedSchema;
	CalciteConnection calciteConnection;
	Planner planner;
	FrameworkConfig config;
	HepPlanner optimizer;
	Logger logger;
	
	
	public SqlStatementParser() throws Exception {
		SystemConfiguration pdfConfig = SystemConfiguration.getInstance();
		config = pdfConfig.getCalciteConfiguration();
		calciteConnection = pdfConfig.getCalciteConnection();
		sharedSchema = pdfConfig.getPdfSchema();
		logger = pdfConfig.getLogger();
		
		planner = new PlannerImpl(config);
		 
		// configure optimizer
		   HepProgramBuilder builder = new HepProgramBuilder();
		  
		    builder.addRuleClass(ReduceExpressionsRule.class);
		    builder.addRuleClass(FilterJoinRule.class);
		    builder.addRuleClass(JoinPushTransitivePredicatesRule.class);
		    builder.addRuleClass(ProjectMergeRule.class);
		    builder.addCommonRelSubExprInstruction(); 
		    builder.addRuleClass(AggregateExpandDistinctAggregatesRule.class);
		    builder.addRuleClass(ProjectToWindowRule.class);
		    builder.addRuleClass(SortProjectTransposeRule.class);
		    builder.addRuleClass(SortJoinTransposeRule.class);
		    builder.addRuleClass(FilterMergeRule.class);
		    builder.addRuleClass(ProjectWindowTransposeRule.class);
		    builder.addRuleClass(FilterProjectTransposeRule.class);
		    builder.addRuleClass(SubQueryRemoveRule.class); // removes EXISTS subqueries
		    //builder.addRuleClass(PushDownFilter.class);
		    
		    
		    
		    optimizer = new HepPlanner(builder.build());
		
		    optimizer.addRule(ProjectToWindowRule.PROJECT);
		    optimizer.addRule(ReduceExpressionsRule.FILTER_INSTANCE);
		    optimizer.addRule(ReduceExpressionsRule.CALC_INSTANCE);
		    optimizer.addRule(ReduceExpressionsRule.PROJECT_INSTANCE);
		    optimizer.addRule(ReduceExpressionsRule.JOIN_INSTANCE);
		    optimizer.addRule(FilterProjectTransposeRule.INSTANCE);

		    optimizer.addRule(FilterJoinRule.FILTER_ON_JOIN);
		    optimizer.addRule(FilterJoinRule.JOIN);
		    optimizer.addRule(JoinPushTransitivePredicatesRule.INSTANCE);
		    
		    optimizer.addRule(AggregateExpandDistinctAggregatesRule.INSTANCE);
		    optimizer.addRule(SortProjectTransposeRule.INSTANCE);
		    optimizer.addRule(FilterTableScanRule.INSTANCE);
		    optimizer.addRule(ProjectWindowTransposeRule.INSTANCE);
		    
		    optimizer.addRule(FilterMergeRule.INSTANCE);
		    optimizer.addRule(ProjectMergeRule.INSTANCE);
		    //optimizer.addRule(SubQueryRemoveRule.FILTER);
		    //optimizer.addRule(SubQueryRemoveRule.JOIN);
		    //optimizer.addRule(SubQueryRemoveRule.PROJECT);
		    //optimizer.addRule(PushDownFilter.INSTANCE);
		    
	}
	
	
	public SqlNode parseSQL(String sql) throws SqlParseException, ValidationException  {
		SqlNode parsed = null;
		

		((PlannerImpl) planner).close();
		((PlannerImpl) planner).reset(); // clear the state from the last parse
		parsed =  planner.parse(sql);
		parsed = planner.validate(parsed);
		
		return parsed;
	}
	
	
	public RelDataTypeFactory getTypeFactory() {
		return planner.getTypeFactory();
	}
	public RelRoot compile(SqlNode sqlRoot) throws RelConversionException {
		RelRoot root =  planner.rel(sqlRoot);
		
		return root;
	}
	
	public FrameworkConfig getConfig() { 
		return config;
	}
	
	public Planner getPlanner() {
		return planner;
	}
	
	// method for minimizing the fields at each RelNode 
	// this reduces the permissions we need to run many operators
	// generalization of SqlToRelTestBase
	  public RelRoot convertSqlToRelMinFields(String sql) throws SqlParseException {
	      assert(sql != null);

		  if(sql.contains(";")) {
			  sql = sql.substring(0, sql.indexOf(';'));
		  }


	      final SqlNode sqlQuery = planner.parse(sql);
	      final RelDataTypeFactory typeFactory = planner.getTypeFactory();
	     

	      final Prepare.CatalogReader catalogReader = new CalciteCatalogReader(
	    	        CalciteSchema.from(sharedSchema),
	    	        CalciteSchema.from(sharedSchema).path(null),
	    	        (JavaTypeFactory) typeFactory, calciteConnection.config());

	          
	      final SqlValidator validator = new LocalValidatorImpl(config.getOperatorTable(), catalogReader, typeFactory,
	              conformance());
	      validator.setIdentifierExpansion(true);
	      
	    
	      final SqlToRelConverter converter =
	          createSqlToRelConverter(
	              validator,
	              catalogReader,
	              typeFactory);
	      
	      final SqlNode validatedQuery = validator.validate(sqlQuery);
	      RelRoot root =
	          converter.convertQuery(validatedQuery, false, true);
	      assert(root != null);

		  String plan = RelOptUtil.dumpPlan("", root.rel, SqlExplainFormat.TEXT, SqlExplainLevel.ALL_ATTRIBUTES);
		  logger.fine("Initial parsed plan:\n" + plan);
	      
	      final boolean ordered = !root.collation.getFieldCollations().isEmpty();
	      
	      RelNode trimmed = converter.trimUnusedFields(ordered, root.rel);



	      root = root.withRel(trimmed);
	      return root;
	    }

	
	  // from PlannerImpl, here b/c of protected method
	  private SqlConformance conformance() {
		    final Context context = config.getContext();
		    if (context != null) {
		      final CalciteConnectionConfig connectionConfig =
		          context.unwrap(CalciteConnectionConfig.class);
		      if (connectionConfig != null) {
		        return connectionConfig.conformance();
		      }
		    }
		    return calciteConnection.config().conformance();
		  }

	  
	// use optimizer to get operators into a canonical form
	// very basic optimizer
	public RelRoot optimize(RelRoot relRoot) {
		
		     optimizer.setRoot(relRoot.rel);
		 		    
		    RelNode out = optimizer.findBestExp();
		    logger.fine("after optimizer: " + RelOptUtil.dumpPlan("", out, SqlExplainFormat.TEXT, SqlExplainLevel.ALL_ATTRIBUTES) );
		    return RelRoot.of(out, relRoot.kind);

	}

	
	public RelRoot trimFields(RelRoot root)  {

	      final RelDataTypeFactory typeFactory = planner.getTypeFactory();
	     

	      final Prepare.CatalogReader catalogReader = new CalciteCatalogReader(
	    	        CalciteSchema.from(sharedSchema),
	    	        CalciteSchema.from(sharedSchema).path(null),
	    	        (JavaTypeFactory) typeFactory, calciteConnection.config());
;

	          
	      final SqlValidator validator = new LocalValidatorImpl(config.getOperatorTable(), catalogReader, typeFactory,
	              conformance());
	      validator.setIdentifierExpansion(true);
	      
	      final SqlToRelConverter converter =
	          createSqlToRelConverter(
	              validator,
	              catalogReader,
	              typeFactory);
	      
	      
	      final boolean ordered = !root.collation.getFieldCollations().isEmpty();
	      RelNode trimmed = converter.trimUnusedFields(ordered, root.rel);
		logger.fine("after trim fields: " + RelOptUtil.dumpPlan("", trimmed, SqlExplainFormat.TEXT, SqlExplainLevel.ALL_ATTRIBUTES) );

	      return root.withRel(trimmed);
	      
	      
	}
	
	
	
    public SqlValidator getValidator() {
        final RelDataTypeFactory typeFactory = planner.getTypeFactory();


         final Prepare.CatalogReader catalogReader = new CalciteCatalogReader(
                   CalciteSchema.from(sharedSchema),
                   CalciteSchema.from(sharedSchema).path(null),
                   (JavaTypeFactory) typeFactory, calciteConnection.config());


         final SqlValidator validator = new LocalValidatorImpl(config.getOperatorTable(), catalogReader, typeFactory,
                 conformance());
         validator.setIdentifierExpansion(true);

     return new LocalValidatorImpl(config.getOperatorTable(), catalogReader, typeFactory,
         conformance());
}
    
	
	public  RexNode parseTableConstraint(String tableName, String predicate) throws Exception {
		String sqlQuery = "SELECT * FROM " + tableName + " WHERE " + predicate;

		SqlNode query = parseSQL(sqlQuery);
        
        
        
        final SqlValidator validator = this.getValidator();
        
        
        
        RelDataTypeSystem typeSystem = PostgresqlSqlDialect.DEFAULT.getTypeSystem();
        final RelDataTypeFactory typeFactory = new JavaTypeFactoryImpl(typeSystem);

            
        
        final CatalogReader catalogReader = new CalciteCatalogReader(
        CalciteSchema.from(sharedSchema),
        CalciteSchema.from(sharedSchema).path(null),
        (JavaTypeFactory) typeFactory, calciteConnection.config());

        assert(typeFactory != null);
        
        SqlToRelConverter sqlToRel = this.createSqlToRelConverter(validator, catalogReader, typeFactory);
        
        
        RelRoot queryTree = sqlToRel.convertQuery(query, true, true);
        RelNode rootNode = queryTree.rel;
        LogicalFilter filter = (LogicalFilter) rootNode.getInput(0); // has to be a filter owing to SELECT setup above
        RexNode rowExpression = filter.getCondition();

        RexBuilder builder = filter.getCluster().getRexBuilder();
        rowExpression = RexUtil.toCnf(builder, rowExpression);
        return rowExpression;
        
        

        
		
	}
	
	
	public RelRoot mergeProjects(RelRoot root) {
		   HepProgramBuilder builder = new HepProgramBuilder();
		   builder.addRuleClass(ProjectMergeRule.class);
		   builder.addRuleClass(ProjectRemoveRule.class);
		   HepPlanner merger = new HepPlanner(builder.build());
		    merger.addRule(ProjectMergeRule.INSTANCE);
		    merger.addRule(ProjectRemoveRule.INSTANCE);
		    merger.setRoot(root.project());

		    RelNode out = merger.findBestExp();

			logger.fine("After merge: " +  RelOptUtil.dumpPlan("", out, SqlExplainFormat.TEXT, SqlExplainLevel.ALL_ATTRIBUTES) );
		    return RelRoot.of(out, root.kind);
		
	}	
	
	public RelRoot logicalCalc(RelRoot root) {
		   HepProgramBuilder builder = new HepProgramBuilder();
		    builder.addRuleClass(FilterToCalcRule.class);
		    builder.addRuleClass(ProjectToCalcRule.class);
		    builder.addRuleClass(FilterCalcMergeRule.class);
		    builder.addRuleClass(ProjectCalcMergeRule.class);
		    builder.addRuleClass(CalcMergeRule.class);

		    
		    HepPlanner calcMaker = new HepPlanner(builder.build());
		    calcMaker.addRule(FilterToCalcRule.INSTANCE);
		    calcMaker.addRule(ProjectToCalcRule.INSTANCE);
		    calcMaker.addRule(FilterCalcMergeRule.INSTANCE);
		    calcMaker.addRule(ProjectCalcMergeRule.INSTANCE);
		    calcMaker.addRule(CalcMergeRule.INSTANCE);
		    
		    calcMaker.setRoot(root.project());

		    RelNode out = calcMaker.findBestExp();
		    return RelRoot.of(out, root.kind);
		
	}
	
	
	
	
    protected SqlToRelConverter createSqlToRelConverter(
            final SqlValidator validator,
            final Prepare.CatalogReader catalogReader,
            final RelDataTypeFactory typeFactory) {
          final RexBuilder rexBuilder = new RexBuilder(typeFactory);

          RelOptCluster cluster =
              RelOptCluster.create(optimizer, rexBuilder);
          
          final SqlToRelConverter.ConfigBuilder configBuilder =  SqlToRelConverter.configBuilder().withTrimUnusedFields(true).withDecorrelationEnabled(true);
          Config config = configBuilder.build();
          return new SqlToRelConverter(null, validator, catalogReader, cluster, StandardConvertletTable.INSTANCE, config);
        }


	
	private class LocalValidatorImpl extends SqlValidatorImpl {
	    protected LocalValidatorImpl(
	        SqlOperatorTable opTab,
	        SqlValidatorCatalogReader catalogReader,
	        RelDataTypeFactory typeFactory,
	        SqlConformance conformance) {
	      super(opTab, catalogReader, typeFactory, conformance);
	    }

	}



	public SqlCreateTable parseTableDefinition(String tableDefinition) throws SqlParseException {
        assert(tableDefinition != null);

        SqlParser.Config sqlParserConfig = SqlParser.configBuilder()
                              .setParserFactory(SqlDdlParserImpl.FACTORY)
                              .setConformance(SqlConformanceEnum.MYSQL_5) // psql not available - blasphemy!
                              .setLex(Lex.MYSQL)
                              .build();
            
        
        SqlParser parser = SqlParser.create(tableDefinition, sqlParserConfig);
        SqlNode node =  parser.parseStmt();

        return (SqlCreateTable) node;
	};
	
}
