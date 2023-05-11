import { ComponentFixture, TestBed } from '@angular/core/testing';

import { CaptureBarComponent } from './capture-bar.component';

describe('CaptureBarComponent', () => {
  let component: CaptureBarComponent;
  let fixture: ComponentFixture<CaptureBarComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ CaptureBarComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(CaptureBarComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
