function DynamicLineChart(container) {
        // set the dimensions and margins of the graph
        this.margin = {top: 10, right: 30, bottom: 30, left: 60},
        this.width = 460 - this.margin.left - this.margin.right,
        this.height = 400 - this.margin.top - this.margin.bottom;
        
        // append the svg object to the body of the page
        this.svg = d3.select("#" + container)
          .append("svg")
            .attr("width", this.width + this.margin.left + this.margin.right)
            .attr("height", this.height + this.margin.top + this.margin.bottom)
          .append("g")
            .attr("transform",
                  "translate(" + this.margin.left + "," + this.margin.top + ")");
        
        /*var lineKey = "name";
        var xKey = "year";
        var yKey = "n";*/
        this.xAxis = null;
        this.yAxis = null;

}

DynamicLineChart.prototype.updateData = function(samples, lineKey, xKey, yKey) {
    var data = samples;
    //var data = [{year:1234, name:"toast", n:1234}, {year:1235, name:"toast", n:2346}, {year:1235, name:"abc", n:200}, {year:1234, name:"abc", n:300}];
    //Read the data
    //d3.csv("https://raw.githubusercontent.com/holtzy/data_to_viz/master/Example_dataset/5_OneCatSevNumOrdered.csv", function(data) {
    
      // group the data: I want to draw one line per group
      var sumstat = d3.nest() // nest function allows to group the calculation per level of a factor
        .key(function(d) { return lineKey(d);})
        .entries(data);
    
      // Add X axis --> it is a date format
      var x = d3.scaleLog()
        .domain(d3.extent(data, function(d) { return +xKey(d); }))
        .range([ 0, this.width ]);
      if (!this.xAxis) {
        this.xAxis = this.svg.append("g")
          .attr("transform", "translate(0," + this.height + ")")
          .call(d3.axisBottom(x).ticks(5));
        }
      else {
        this.xAxis.call(d3.axisBottom(x))
      }
    
      // Add Y axis
      var y = d3.scaleLinear()
        .domain([0, d3.max(data, function(d) { return +yKey(d); })])
        .range([ this.height, 0 ]);
      if (!this.yAxis) {
        this.yAxis = this.svg.append("g")
          .call(d3.axisLeft(y));
      }
      else {
        this.yAxis.call(d3.axisLeft(y))
      }

      // color palette
      var res = sumstat.map(function(d){ return d.key }) // list of group names
      var color = d3.scaleOrdinal()
        .domain(res)
        .range(['#e41a1c','#377eb8','#4daf4a','#984ea3','#ff7f00','#ffff33','#a65628','#f781bf','#999999'])
    
      this.svg.selectAll(".line")
        .data(sumstat)
        .exit()
        .remove()

      //console.log(sumstat);

      this.svg.selectAll(".line")
          .data(sumstat)
          .exit()
          .remove();

      // Draw the line
      this.svg.selectAll(".line")
          .data(sumstat)
          .enter()
          .append("path")
            .attr("class","line")
            .attr("fill", "none")
            .attr("stroke", function(d){ return color(d.key) })
            .attr("stroke-width", 1.5)
            .attr("d", function(d){
              return d3.line()
                .x(function(d) { return x(xKey(d)); })
                .y(function(d) { return y(+yKey(d)); })
                (d.values)
            });

      this.svg.selectAll(".line")
          .data(sumstat)
          .attr("d", function(d){
              return d3.line()
                .x(function(d) { return x(xKey(d)); })
                .y(function(d) { return y(+yKey(d)); })
                (d.values)
            });
  }





