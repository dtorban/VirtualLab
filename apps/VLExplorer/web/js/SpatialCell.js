function SpatialCell(container, showPath, selectCell) {
  this.container = container;
  // set the dimensions and margins of the graph
  this.margin = {top: 10, right: 30, bottom: 30, left: 60},
  this.width = container.width() - this.margin.left - this.margin.right,
  this.height = container.height() - this.margin.top - this.margin.bottom;

  this.selectCell = selectCell;

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

  this.svg.append("rect")
    .attr("x", 0)
    .attr("x", 0)
    .attr("width", this.width)
    .attr("height", this.height)
    .attr("fill", "white")
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
        .attr("stroke-width", "2")
  
      this.svg.selectAll("line")
      .attr("fill", function(d) {return d.color;})
      .attr("stroke", function(d) {return d.color;})
      .attr('x1', function(d) {return self.width/2/gridWidth + d.gridX*self.width/gridWidth;})
      .attr('y1', function(d) {return self.height/2/gridHeight + d.gridY*self.height/gridHeight;})
      .attr('x2', function(d) {return scale*(d.arm.x-d.x)/50.0 + self.width/2/gridWidth + d.gridX*self.width/gridWidth;})
      .attr('y2', function(d) {return scale*(d.arm.y-d.y)/50.0 + self.height/2/gridHeight + d.gridY*self.height/gridHeight;})

  }
  else {

    /*var dist = this.svg.selectAll('circle.dist')
      .data(data);
      
      dist.exit()
      .remove()

      dist.enter()
      .append('circle')
      .attr('class', 'dist')
      .attr("stroke", 'black')
      .attr("fill", 'none')
    .exit()
      .remove()
    .merge( dist )
        .attr('cx', function(d) { return x(xKey(d)); })
        .attr('cy', function(d) { return y(+yKey(d)); });*/


    var path = [];
    var maxLength = 0;
    for (var i = 0; i < data.length; i++) {
      var h = [];
      if (data[i].data.h) {
        for (var j = 0; j < data[i].data.h.length; j++) {
          var hCalc = {x: data[i].data.h[j].x - data[i].data.h[0].x, y: data[i].data.h[j].y - data[i].data.h[0].y, gridX: i%gridWidth, gridY: Math.floor(i/gridWidth)};
          var length = Math.sqrt(hCalc.x*hCalc.x + hCalc.y*hCalc.y);
          if (length > maxLength) {
            maxLength = length;
          }
          h.push(hCalc);
        }
      }
      path.push({color: data[i].color, h: h});
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

  var rows = [];
  var grid = [];
  for (var j = 0; j < gridHeight; j++) {
    for (var i = 0; i < gridWidth; i++) {
      grid.push({x: i, y: j, data: data[j*gridWidth+i]});
    }
    rows.push({y:j});
  }

  /*var selectRows = this.svg.selectAll(".selectRows");
  selectRows.data(rows)
    .exit()
    .remove();
  selectRows.data(rows)
    .enter()
      .append('rect')
      .attr("class","selectRows")
      .attr("stroke-width", 2)
      .attr("stroke", "red")
      .attr("fill", "none")
      //.attr("stroke-dasharray","2,5")
      .style("opacity", 0.5)
    .merge(selectRows)
      .attr("x", function(d) {return 0})
      .attr("y", function(d) {return d.y*self.height/gridHeight+5})
      .attr("width", function(d) {return self.width})
      .attr("height", function(d) {return self.height/gridHeight-5})*/

  var selectRects = this.svg.selectAll(".selectRects");
  selectRects.data(grid)
    .exit()
    .remove();
  selectRects.data(grid)
    .enter()
      .append('rect')
      .attr("class","selectRects")
      .attr("stroke-width", 3)
      .attr("stroke", "blue")
      .attr("fill", "blue")
      .attr("fill-opacity", 0.0)
      .style("opacity", 0.0)
      .style('cursor', 'zoom-in')
      .on("mouseenter", function(d) {
        d3.select(d3.event.target)
          .style("opacity","1.0");
          this.parentNode.appendChild(this);
      })
      .on("mouseleave", function(d) {
        d3.select(d3.event.target)
          .style("opacity","0.0");
      })
      .on("mousedown", function(d) {
        if (self.selectCell) {
          self.selectCell(d.x, d.y, d.data);
        }
      })
    .merge(selectRects)
      .attr("x", function(d) {return d.x*self.width/gridWidth+2})
      .attr("y", function(d) {return d.y*self.height/gridHeight+2})
      .attr("width", function(d) {return self.width/gridWidth-2})
      .attr("height", function(d) {return self.height/gridHeight-2})
      

}





