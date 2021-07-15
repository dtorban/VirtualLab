function SpatialCell(container, showPath) {
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
  this.showPath = showPath;
}

SpatialCell.prototype.updateData = function(data, gridWidth, gridHeight) {
  var self = this;

  var scale = gridWidth > gridHeight ? 1.0/gridWidth : 1.0/gridHeight;

  if (!this.showPath) {
    var maxLength = 0;
    var arms = [];
    for (var i = 0; i < data.length; i++) {
      for (var j = 0; j < data[i].data.m.length; j++) {
        var arm = {gridX: i%gridWidth, gridY: Math.floor(i/gridWidth), color: data[i].color, arm: data[i].data.m[j], x: data[i].data.x, y: data[i].data.y};
        arms.push(arm)
        var length = Math.sqrt((arm.arm.x - arm.x)*(arm.arm.x - arm.x) + (arm.arm.y - arm.y)*(arm.arm.y - arm.y));
        if (length > maxLength) {
          maxLength = length;
        }
      }
    }

    scale = 20000*scale/maxLength;
  
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
  else {
    var path = [];
    var maxLength = 0;
    for (var i = 0; i < data.length; i++) {
      var h = [];
      for (var j = 0; j < data[i].data.h.length; j++) {
        var hCalc = {x: data[i].data.h[j].x - data[i].data.h[0].x, y: data[i].data.h[j].y - data[i].data.h[0].y, gridX: i%gridWidth, gridY: Math.floor(i/gridWidth)};
        var length = Math.sqrt(hCalc.x*hCalc.x + hCalc.y*hCalc.y);
        if (length > maxLength) {
          maxLength = length;
        }
        h.push(hCalc);
      }
      path.push({color: data[i].color, h: h})
    }

    scale = 20*scale/maxLength;

    this.svg.selectAll(".line")
      .data(path)
      .exit()
      .remove();

    // Draw the line
    this.svg.selectAll(".line")
        .data(path)
        .enter()
        .append("path")
          .attr("class","line")
          .attr("fill", "none")
          .attr("stroke", function(d){ return d.color; })
          .style("opacity", 1.0)
          //.style("opacity", function(d) { return +d.values[0].chosen > 0 ? 0 : 1; })
          //.attr("stroke-width", function(d) { return +d.values[0].chosen*2.0 + 1.5; })
          .attr("stroke-width", function(d) { return 4; })
          .attr("d", function(d){
            return d3.line()
              .x(function(d) { return scale*(d.x)*10 + self.width/2/gridWidth + d.gridX*self.width/gridWidth; })
              .y(function(d) { return scale*(d.y)*10 + self.height/2/gridHeight + d.gridY*self.height/gridHeight; })
              (d.h)
            })


  this.svg.selectAll(".line")
      .data(path)
      //.style("opacity", function(d) { return +d.values[0].chosen > 0 ? 0 : 1; })
          //.attr("stroke-width", function(d) { return +d.values[0].chosen*2.0 + 1.5; })
          .attr("stroke-width", function(d) { return 1.5; })
          .attr("stroke", function(d){ return d.color; })
      .attr("d", function(d){
        return d3.line()
          .x(function(d) { return scale*(d.x)*20 + self.width/2/gridWidth + d.gridX*self.width/gridWidth; })
          .y(function(d) { return scale*(d.y)*20 + self.height/2/gridHeight + d.gridY*self.height/gridHeight; })
          (d.h)
        })
  }
  
}





