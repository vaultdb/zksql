WITH q1_projection AS (
    SELECT l_returnflag, l_linestatus, l_quantity, l_discount, l_extendedprice, l_extendedprice * (1.0 - l_discount) AS disc_price, l_extendedprice * (1.0 - l_discount) * (1.0 + l_tax) AS charge, l_shipdate
    FROM lineitem
    ORDER BY l_returnflag, l_linestatus)
SELECT
     l_returnflag,
     l_linestatus,
     SUM(l_quantity) as sum_qty,
     SUM(l_extendedprice) as sum_base_price,
     SUM(disc_price) as sum_disc_price,
     SUM(charge) as sum_charge,
     AVG(l_quantity) as avg_qty,
     AVG(l_extendedprice) as avg_price,
     AVG(l_discount) as avg_disc,
     COUNT(*) as count_order
FROM
    q1_projection
WHERE
     l_shipdate <= date '1998-08-03'
GROUP BY
   l_returnflag,
   l_linestatus
ORDER BY
    l_returnflag,
    l_linestatus;