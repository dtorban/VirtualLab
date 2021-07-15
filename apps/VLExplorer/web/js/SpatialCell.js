function SpatialCell(container) {
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
}

SpatialCell.prototype.updateData = function(data, gridWidth, gridHeight) {
  var self = this;

  var arms = [];
  for (var i = 0; i < data.length; i++) {
    for (var j = 0; j < data[i].data.m.length; j++) {
      arms.push({gridX: i%gridWidth, gridY: Math.floor(i/gridWidth), color: data[i].color, arm: data[i].data.m[j], x: data[i].data.x, y: data[i].data.y})
    }
  }

  var scale = gridWidth > gridHeight ? 1.0/gridWidth : 1.0/gridHeight;

  //var arms = data.m;
  this.svg.selectAll("line")
    .data(arms)
    .exit()
    .remove()
    this.svg.selectAll("line")
    .data(arms)
    .enter()
    .append("line")
      .attr('x1', function(d) {return self.width/2/gridWidth + d.gridX*self.width/gridWidth;})
      .attr('y1', function(d) {return self.height/2/gridHeight + d.gridY*self.height/gridHeight;})
      .attr('x2', function(d) {return scale*(d.arm.x-d.x)/50.0 + self.width/2/gridWidth + d.gridX*self.width/gridWidth;})
      .attr('y2', function(d) {return (d.arm.y-d.y)/50.0 + self.height/2/gridHeight + d.gridY*self.height/gridHeight;})
      .attr("fill", function(d) {return d.color;})
      .attr("stroke", function(d) {return d.color;})
      //.attr("stroke-opacity", "0.3")
      .attr("stroke-width", "5")

    this.svg.selectAll("line")
    .attr("fill", function(d) {return d.color;})
    .attr("stroke", function(d) {return d.color;})
    .attr('x1', function(d) {return self.width/2/gridWidth + d.gridX*self.width/gridWidth;})
    .attr('y1', function(d) {return self.height/2/gridHeight + d.gridY*self.height/gridHeight;})
    .attr('x2', function(d) {return scale*(d.arm.x-d.x)/50.0 + self.width/2/gridWidth + d.gridX*self.width/gridWidth;})
    .attr('y2', function(d) {return scale*(d.arm.y-d.y)/50.0 + self.height/2/gridHeight + d.gridY*self.height/gridHeight;})
  }





