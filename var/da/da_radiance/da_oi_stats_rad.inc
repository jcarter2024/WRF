subroutine da_oi_stats_rad ( stats_unit, iv )

   !---------------------------------------------------------------------------
   ! Purpose: Calculate statistics of obs minus background for radiance data.
   !
   ! METHOD:  average, rms, minimum, maximum of iv
   !---------------------------------------------------------------------------

   implicit none

   integer,        intent (in)      :: stats_unit    ! Output unit for stats.
   type (iv_type), intent (in)      :: iv            ! Innovation

   type (stats_rad_type),    pointer  :: rad(:)
   integer                          :: n, k, i

   if (trace_use) call da_trace_entry("da_oi_stats_rad")

   allocate ( rad(1:iv%num_inst) )

   do i = 1, iv%num_inst                          !! loop for sensors
      allocate ( rad(i)%ichan(1:iv%instid(i)%nchan) )
      rad(i)%ichan(1:iv%instid(i)%nchan)%num  = 0
      rad(i)%ichan(1:iv%instid(i)%nchan)%ave  = 0.0
      rad(i)%ichan(1:iv%instid(i)%nchan)%rms  = 0.0
      rad(i)%ichan(1:iv%instid(i)%nchan)%minimum%value  = -missing_r
      rad(i)%ichan(1:iv%instid(i)%nchan)%maximum%value  =  missing_r
      rad(i)%ichan(1:iv%instid(i)%nchan)%minimum%n      = 1
      rad(i)%ichan(1:iv%instid(i)%nchan)%maximum%n      = 1
      do k=1,iv%instid(i)%nchan
         rad(i)%ichan(k)%minimum%l      = k
         rad(i)%ichan(k)%maximum%l      = k
      end do

      if (iv%instid(i)%num_rad < 1) cycle
      do k=1, iv%instid(i)%nchan               !! loop for channels
         do n=1, iv%instid(i)%num_rad              !! loop for pixels
            if (iv%instid(i)%info%proc_domain(1,n)) then
               call da_stats_calculate( n,k,iv%instid(i)%tb_qc(k,n), &
                  iv%instid(i)%tb_inv(k,n), rad(i)%ichan(k)%num, &
                  rad(i)%ichan(k)%minimum, rad(i)%ichan(k)%maximum, &
                  rad(i)%ichan(k)%ave, rad(i)%ichan(k)%rms)

            end if          ! end if( iv%sound(n)%loc%proc_domain )
         end do                                 !! end loop for pixels
      end do                        !  end loop for channels
   end do                         !  end loop for sensor

   do i = 1, iv%num_inst                          !! loop for sensors
      do k=1, iv%instid(i)%nchan               !! loop for channels
         ! Do inter-processor communication to gather statistics.
         call da_proc_sum_int ( rad(i)%ichan(k)%num )
         call da_proc_stats_combine(rad(i)%ichan(k)%ave, rad(i)%ichan(k)%rms, &
            rad(i)%ichan(k)%minimum%value, rad(i)%ichan(k)%maximum%value, &
            rad(i)%ichan(k)%minimum%n, rad(i)%ichan(k)%maximum%n, &
            rad(i)%ichan(k)%minimum%l, rad(i)%ichan(k)%maximum%l )

      end do                        !  end loop for channels

      if (rootproc) then
         if ( any( rad(i)%ichan(:)%num /= 0 ) ) then
            write(unit=stats_unit, fmt='(/a/)') &
               ' Diagnostics of OI for radiance         '//iv%instid(i)%rttovid_string 
            call da_print_stats_rad( stats_unit, iv%instid(i)%nchan, rad(i) )
         end if
      end if
   end do                         !  end loop for sensor

   do i = 1, iv%num_inst           ! loop for sensors
      deallocate (rad(i)%ichan)
   end do

   deallocate (rad)

   if (trace_use) call da_trace_exit("da_oi_stats_rad")

end subroutine da_oi_stats_rad


