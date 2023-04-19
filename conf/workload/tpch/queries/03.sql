SELECT
  l.l_orderkey,
  SUM(revenue) as revenue,
  o.o_orderdate,
  o.o_shippriority
FROM
  customer c,
  orders o,
  (SELECT l_orderkey, l_shipdate, l_extendedprice * (1.0 - l_discount) revenue FROM lineitem) l
WHERE
  c.c_mktsegment = 'HOUSEHOLD'
  and c.c_custkey = o.o_custkey
  and l.l_orderkey = o.o_orderkey
  and o.o_orderdate < date '1995-03-25'
  and l.l_shipdate > date '1995-03-25'
GROUP BY
  l.l_orderkey,
  o.o_orderdate,
  o.o_shippriority
ORDER BY
    revenue desc,
  o.o_orderdate
limit 10;