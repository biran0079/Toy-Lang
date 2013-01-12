#include "dumpGCHistory.h"
#include "gc.h"
#include "list.h"
#include <stdio.h>

extern int newNodeC, newIntValueC, newStringValueC, newClosureValueC, newEnvValueC,
    newListValueC, newClosureC, newEnvC, newBuiltinFunC;
extern List* gcHistory;

void dumpGCHistory(FILE* out) {
fprintf(out,
"<html>"
"  <head>"
"    <script type='text/javascript' src='https://www.google.com/jsapi'></script>"
"    <script type='text/javascript'>"
"      google.load('visualization', '1', {packages:['corechart']});"
"      google.setOnLoadCallback(drawChart);"
"      function drawChart() {"
"        var data = google.visualization.arrayToDataTable(["
"          ['Value Type', 'How many'],"
"          ['Int',     %d],"
"          ['String',      %d],"
"          ['Closure',  %d],"
"          ['Env', %d],"
"          ['List',    %d],"
"          ['BuiltinFunc',    %d],"
"        ]);", newIntValueC, newStringValueC, newClosureValueC, newEnvValueC, newListValueC, newBuiltinFunC);
fprintf(out, 
"        var options = {"
"          title: 'Values Distribution'"
"        };"
"        var chart = new google.visualization.PieChart(document.getElementById('distribution'));"
"        chart.draw(data, options);"
""
"        data = google.visualization.arrayToDataTable(["
"          ['GC', 'newed', 'freed'],");
int i = 0;
fprintf(out, "['0' , 0 , 0],\n");
for(i = 0; i<listSize(gcHistory); i++) {
  GCRecord* r = listGet(gcHistory, i);
  fprintf(out, "['%d' , %d , %d],\n", i+1, r->before, r->after);
}
fprintf(out, 
"        ]);"
"        var options = {"
"          title: 'Values',"
"        };"
"        chart = new google.visualization.AreaChart(document.getElementById('values'));"
"        chart.draw(data, options);"
"      }"
"    </script>"
"  </head>"
"  <body>"
"    <div id='distribution' style='width: 900px; height: 500px;'></div>"
"    <div id='values' style='width: 900px; height: 500px;'></div>"
"  </body>"
"</html>");
}
