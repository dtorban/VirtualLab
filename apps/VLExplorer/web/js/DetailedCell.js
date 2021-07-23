function DetailedCell(container) {
  this.container = container;
  // set the dimensions and margins of the graph
  this.margin = {top: 10, right: 30, bottom: 30, left: 60},
  this.width = container.width() - this.margin.left - this.margin.right,
  this.height = container.height() - this.margin.top - this.margin.bottom;

  // append the svg object to the body of the page
  this.svg = d3.select(container[0])
    .append("svg")
      .attr("width", this.width + this.margin.left + this.margin.right)
      .attr("height", this.height + this.margin.top + this.margin.bottom)
      //.style("background-color", "#D8D8D8")
    .append("g")
      .attr("transform",
            "translate(" + this.margin.left + "," + this.margin.top + ")");

    this.x = d3.scaleLinear()
            .domain([0,1])
            .range([ 0, this.width ]);
    
    this.y = d3.scaleLinear()
            .domain([0,1])
            .range([ this.height, 0]);
    
}

DetailedCell.prototype.updateData = function(data) {
  var self = this;

  this.x.domain([-5000, 5000]);
  this.y.domain([-5000, 5000]);

  this.svg.selectAll("circle")
    .data([data])
    .enter()
      .append("circle")
      .style("stroke", "blue")
      .style("fill", "blue")
      .attr("r", 40)
    .merge(this.svg.selectAll("circle"))
      .attr("cx", function(d, i){console.log(self.x(+d.data.x)); return self.x(+d.data.x);})
      .attr("cy", function(d, i){return self.y(+d.data.y);});

  console.log(data);
}





