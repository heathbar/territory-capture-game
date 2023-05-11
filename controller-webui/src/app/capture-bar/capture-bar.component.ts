import { Component, Input } from '@angular/core';

@Component({
  selector: 'app-capture-bar',
  templateUrl: './capture-bar.component.html',
  styleUrls: ['./capture-bar.component.scss']
})
export class CaptureBarComponent {

  @Input() red: number = 0;
  @Input() blue: number = 0;
  @Input() scoreToWin = 600;
  
  get redValue() {
    return this.red * 100 / 600;
  }

  get blueValue() {
    return this.blue * 100 / 600;
  }
}
