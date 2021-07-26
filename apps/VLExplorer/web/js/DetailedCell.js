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
            
    this.thickness = 80;
    
    /*this.clip = this.svg.append("defs").append("clipPath")
        .attr("id", "round-corner")
        .append("rect")
          .attr("width", 10 )
          .attr("height", 10 )
          .attr("x", 0)
          .attr("y", 0);*/

}

DetailedCell.prototype.calcArmLength = function(x1, y1, x2, y2) {
  return Math.sqrt(Math.pow(this.x(x1)-this.x(x2),2) + Math.pow(this.y(y1)-this.y(y2),2));
}

DetailedCell.prototype.updateData = function(data) {
  var self = this;

  var aspectRatio = 1.0*this.width/this.height;
  
  this.x.domain([-15000*aspectRatio, 15000*aspectRatio]);
  this.y.domain([-15000, 15000]);

  this.svg.selectAll("circle")
    .data([data])
    .enter()
      .append("circle")
      .style("stroke", "blue")
      .style("fill", "blue")
      .attr("r", 5)
    .merge(this.svg.selectAll("circle"))
      .attr("cx", function(d, i){return self.x(+d.data.x);})
      .attr("cy", function(d, i){return self.y(+d.data.y);});

    /*//var arms = data.m;
    this.svg.selectAll("line")
      .data(data.data.m)
      .exit()
      .remove()
      this.svg.selectAll("line")
      .data(data.data.m)
      .enter()
      .append("line")
        .attr("fill", "black")
        .attr("stroke", "black")
        //.attr("stroke-width", "100")
        .attr("rx", "15")
      .merge(this.svg.selectAll("line"))
        .attr('x1', function(d) {return self.x(data.data.x); })
        .attr('y1', function(d) {return self.y(data.data.y); })
        //.attr('x2', function(d) {return self.x(data.data.x + d.x);})
        //.attr('y2', function(d) {return self.y(data.data.y + d.y);})
        .attr('x2', function(d) {return self.x(d.x);})
        .attr('y2', function(d) {return self.y(d.y);})*/

    

    this.svg.selectAll(".rect1")
      .data(data.data.m)
      .exit()
      .remove()
      this.svg.selectAll(".rect1")
      .data(data.data.m)
      .enter()
      .append("rect")
        .attr("class", "rect1")
        .attr("fill", "lightgrey")
        .attr("stroke", "lightgrey")
        //.attr("stroke-width", "100")
        .attr("rx", "50")
        .attr('height', self.thickness)
        //.attr("clip-path", "url(#round-corner)")
      .merge(this.svg.selectAll(".rect1"))
        .attr('x', function(d) {return self.x(data.data.x); })
        .attr('y', function(d) {return self.y(data.data.y); })
        .attr('width', function(d) { return self.calcArmLength(d.x, d.y, data.data.x, data.data.y);})
        .attr('transform', function(d) { 
          var len = self.calcArmLength(d.x, d.y, data.data.x, data.data.y);
          var opposite = d.y-data.data.y;//-1;
          var adjacent = d.x-data.data.x;//1;
          var angle = Math.atan(opposite / adjacent)*360/(2*Math.PI);
          if (adjacent < 0) {
            angle += 180;
          }
          return "translate("+ self.x(data.data.x) +","+ self.y(data.data.y) +") rotate("+(-angle)+") translate(0,"+(-self.thickness/2.0)+") translate("+ (-self.x(data.data.x)) +","+ (-self.y(data.data.y)) +")";
        })

        this.svg.selectAll(".rect2")
        .data(data.data.m)
        .exit()
        .remove()
        this.svg.selectAll(".rect2")
        .data(data.data.m)
        .enter()
        .append("rect")
          .attr("class", "rect2")
          .attr("fill", "lightgrey")
          .attr("stroke", "lightgrey")
          //.attr("stroke-width", "100")
          .attr("rx", "5")
          .attr('height', self.thickness)
          //.attr("clip-path", "url(#round-corner)")
        .merge(this.svg.selectAll(".rect2"))
          .attr('x', function(d) {return self.x(data.data.x); })
          .attr('y', function(d) {return self.y(data.data.y); })
          .attr('width', function(d) { return self.calcArmLength(d.x, d.y, data.data.x, data.data.y)/5.0;})
          .attr('transform', function(d) { 
            var len = self.calcArmLength(d.x, d.y, data.data.x, data.data.y)/5.0;
            var opposite = d.y-data.data.y;//-1;
            var adjacent = d.x-data.data.x;//1;
            var angle = Math.atan(opposite / adjacent)*360/(2*Math.PI);
            if (adjacent < 0) {
              angle += 180;
            }
            return "translate("+ self.x(data.data.x) +","+ self.y(data.data.y) +") rotate("+(-angle)+") translate(0,"+(-self.thickness/2.0)+") translate("+ (-self.x(data.data.x)) +","+ (-self.y(data.data.y)) +")";
          })

        var h = [];
        if (data.data.h) {
          h = data.data.h;
          /*for (var i = 0; i < data.data.h.length; i++) {
            var sumX = 0.0;
            var sumY = 0.0;
            var num = 0;
            for (var j = i-5; j < i+5; j++) {
              if (j >= 0 && j <= data.data.h.length -1) {
                sumX += data.data.h[j].x;
                sumY += data.data.h[j].y;
                num++;
              }
            }
            h.push({x: sumX/num, y: sumY/num});
          }*/
        }

        this.svg.selectAll(".line")
          .data([h])
          .exit()
          .remove();
        this.svg.selectAll(".line")
            .data([h])
            .enter()
            .append("path")
              .attr("class","line")
              .attr("fill", "none")
              .attr("stroke", "blue")
              .style("opacity", 1.0)
              .attr("stroke-width", function(d) { return 10; })
              .attr("d", function(d){
                console.log("data:",d);
                return d3.line()
                  .curve(d3.curveBasis)
                  .x(function(d) { return self.x(d.x*1000); })
                  .y(function(d) { return self.y(d.y*1000); })
                  (d)
                })
            .merge(this.svg.selectAll(".line"))
              //.attr("stroke-width", function(d) { return 1.5; })
              //.attr("stroke", function(d){ return d.color; })
              .attr("d", function(d){
                console.log(d);
                return d3.line()
                  .curve(d3.curveBasis)
                  .x(function(d) { return self.x(d.x*1000); })
                  .y(function(d) { return self.y(d.y*1000); })
                  (d)
                })

            d3.selection.prototype.moveToFront = function() {  
              return this.each(function(){
                this.parentNode.appendChild(this);
              });
            };

            this.svg.selectAll(".line").moveToFront();

  console.log(data);
}





